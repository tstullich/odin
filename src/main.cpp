#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE  // Will need this because OpenGL uses \
                                    // [-1.0, 1.0] z-depth range
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "vk/command_pool.hpp"
#include "vk/descriptor_pool.hpp"
#include "vk/descriptor_set.hpp"
#include "vk/device_manager.hpp"
#include "vk/index_buffer.hpp"
#include "vk/instance.hpp"
#include "vk/pipeline.hpp"
#include "vk/render_pass.hpp"
#include "vk/swapchain.hpp"
#include "vk/texture_image.hpp"
#include "vk/texture_sampler.hpp"
#include "vk/uniform_buffer.hpp"
#include "vk/vertex_buffer.hpp"

const int WIDTH = 800;
const int HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;

const std::string MODEL_PATH = "models/chalet.obj";
const std::string TEXTURE_PATH = "textures/chalet.jpg";

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct Vertex {
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 texCoord;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 3>
  getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
  }

  bool operator==(const Vertex& other) const {
    return pos == other.pos && color == other.color &&
           texCoord == other.texCoord;
  }
};

// Need this for a proper hash function
// https://en.cppreference.com/w/cpp/utility/hash
namespace std {
template <>
struct hash<Vertex> {
  size_t operator()(Vertex const& vertex) const {
    return ((hash<glm::vec3>()(vertex.pos) ^
             (hash<glm::vec3>()(vertex.color) << 1)) >>
            1) ^
           (hash<glm::vec2>()(vertex.texCoord) << 1);
  }
};
}  // namespace std

struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
} ubo;

namespace odin {
class App {
 public:
  void run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

 private:
  GLFWwindow* window;

  std::unique_ptr<odin::Instance> instance;
  VkSurfaceKHR surface;

  std::unique_ptr<DeviceManager> deviceManager;

  std::unique_ptr<Swapchain> swapChain;

  std::unique_ptr<RenderPass> renderPass;

  std::unique_ptr<DescriptorSet> descriptorSetLayout;

  std::unique_ptr<Pipeline> graphicsPipeline;

  std::unique_ptr<CommandPool> commandPool;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  size_t currentFrame = 0;

  bool framebufferResized = false;

  std::vector<Vertex> vertices;
  std::unique_ptr<VertexBuffer> vertexBuffer;

  std::vector<uint32_t> indices;
  std::unique_ptr<IndexBuffer> indexBuffer;

  std::vector<UniformBuffer> uniformBuffers;

  std::unique_ptr<DescriptorPool> descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;

  std::unique_ptr<TextureImage> textureImage;
  VkImageView textureImageView;
  std::unique_ptr<TextureSampler> textureSampler;

  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  void initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Odin", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  }

  void initVulkan() {
    createInstance();
    createSurface();
    createDeviceManager();
    createSwapChain();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool();
    createDepthResources();
    createFrameBuffers();
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    loadModel();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
  }

  void createInstance() {
    instance = std::make_unique<odin::Instance>(enableValidationLayers);
  }

  void mainLoop() {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      drawFrame();
    }

    // Needed so that we do not destroy resources while
    // draw calls may still be going on
    vkDeviceWaitIdle(deviceManager->getLogicalDevice());
  }

  void drawFrame() {
    vkWaitForFences(deviceManager->getLogicalDevice(), 1,
                    &inFlightFences[currentFrame], VK_TRUE,
                    std::numeric_limits<uint64_t>::max());

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        deviceManager->getLogicalDevice(), swapChain->getSwapchain(),
        std::numeric_limits<uint64_t>::max(),
        imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapChain();
      return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      throw std::runtime_error("Failed to acquire swap chain image!");
    }

    updateUniformBuffer(imageIndex);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = commandPool->getCommandBuffer(imageIndex);

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(deviceManager->getLogicalDevice(), 1,
                  &inFlightFences[currentFrame]);

    if (vkQueueSubmit(deviceManager->getGraphicsQueue(), 1, &submitInfo,
                      inFlightFences[currentFrame]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain->getSwapchain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result =
        vkQueuePresentKHR(deviceManager->getPresentationQueue(), &presentInfo);

    // This check should be done after vkQueuePresentKHR to avoid
    // improperly signalled Semaphores
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        framebufferResized) {
      framebufferResized = false;
      recreateSwapChain();
    } else if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  void cleanupSwapChain() {
    //  vkDestroyImageView(deviceManager->getLogicalDevice(), depthImageView,
    //                     nullptr);
    //  vkDestroyImage(deviceManager->getLogicalDevice(), depthImage, nullptr);
    //  vkFreeMemory(deviceManager->getLogicalDevice(), depthImageMemory,
    //  nullptr);

    //  for (auto framebuffer : swapChainFramebuffers) {
    //    vkDestroyFramebuffer(deviceManager->getLogicalDevice(), framebuffer,
    //                         nullptr);
    //  }

    //  vkFreeCommandBuffers(deviceManager->getLogicalDevice(), commandPool,
    //                       static_cast<uint32_t>(commandBuffers.size()),
    //                       commandBuffers.data());

    //  vkDestroyPipeline(deviceManager->getLogicalDevice(), graphicsPipeline,
    //                    nullptr);
    //  vkDestroyPipelineLayout(deviceManager->getLogicalDevice(),
    //  pipelineLayout,
    //                          nullptr);
    //  vkDestroyRenderPass(deviceManager->getLogicalDevice(), renderPass,
    //  nullptr);

    //  for (auto imageView : swapChainImageViews) {
    //    vkDestroyImageView(deviceManager->getLogicalDevice(), imageView,
    //    nullptr);
    //  }

    //  vkDestroySwapchainKHR(deviceManager->getLogicalDevice(), swapChain,
    //                        nullptr);

    //  for (size_t i = 0; i < swapChainImages.size(); i++) {
    //    vkDestroyBuffer(deviceManager->getLogicalDevice(), uniformBuffers[i],
    //                    nullptr);
    //    vkFreeMemory(deviceManager->getLogicalDevice(),
    //    uniformBuffersMemory[i],
    //                 nullptr);
    //  }

    //  vkDestroyDescriptorPool(deviceManager->getLogicalDevice(),
    //  descriptorPool,
    //                          nullptr);
  }

  void cleanup() {
    cleanupSwapChain();

    vkDestroySampler(deviceManager->getLogicalDevice(), textureSampler,
                     nullptr);
    vkDestroyImageView(deviceManager->getLogicalDevice(), textureImageView,
                       nullptr);

    vkDestroyImage(deviceManager->getLogicalDevice(), textureImage, nullptr);
    vkFreeMemory(deviceManager->getLogicalDevice(), textureImageMemory,
                 nullptr);

    vkDestroyDescriptorSetLayout(deviceManager->getLogicalDevice(),
                                 *descriptorSetLayout->getDescriptorSetLayout(),
                                 nullptr);

    vkDestroyBuffer(deviceManager->getLogicalDevice(), indexBuffer, nullptr);
    vkFreeMemory(deviceManager->getLogicalDevice(), indexBufferMemory, nullptr);

    vkDestroyBuffer(deviceManager->getLogicalDevice(), vertexBuffer, nullptr);
    vkFreeMemory(deviceManager->getLogicalDevice(), vertexBufferMemory,
                 nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(deviceManager->getLogicalDevice(),
                         imageAvailableSemaphores[i], nullptr);
      vkDestroySemaphore(deviceManager->getLogicalDevice(),
                         renderFinishedSemaphores[i], nullptr);
      vkDestroyFence(deviceManager->getLogicalDevice(), inFlightFences[i],
                     nullptr);
    }

    vkDestroyCommandPool(deviceManager->getLogicalDevice(),
                         commandPool->getCommandPool(), nullptr);

    vkDestroyDevice(deviceManager->getLogicalDevice(), nullptr);

    vkDestroySurfaceKHR(instance->getInstance(), surface, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
  }

  void createCommandBuffers() {
    commandPool->createCommandBuffers(deviceManager->getLogicalDevice(),
                                      *renderPass, *graphicsPipeline,
                                      *swapChain);
  }

  void createCommandPool() { commandPool = std::make_unique<CommandPool>(); }

  VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(deviceManager->getPhysicalDevice(),
                                          format, &props);

      // More formats are available but we only need linear and optimal tiling
      // for now
      if (tiling == VK_IMAGE_TILING_LINEAR &&
          (props.linearTilingFeatures & features) == features) {
        return format;
      } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                 (props.optimalTilingFeatures & features) == features) {
        return format;
      }
    }

    throw std::runtime_error("Failed to find supported format!");
  }

  VkFormat findDepthFormat() {
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
         VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  }

  void createDepthResources() {
    VkFormat depthFormat = findDepthFormat();

    auto swapChainExtent = swapChain->getExtent();
    createImage(
        swapChainExtent.width, swapChainExtent.height, 1, depthFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);

    depthImageView = swapChain->createImageView(
        deviceManager->getLogicalDevice(), depthImage, depthFormat,
        VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
  }

  void createDescriptorSetLayout() {
    descriptorSetLayout =
        std::make_unique<DescriptorSet>(deviceManager->getLogicalDevice());
  }

  void createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      if (vkCreateSemaphore(deviceManager->getLogicalDevice(), &semaphoreInfo,
                            nullptr,
                            &imageAvailableSemaphores[i]) != VK_SUCCESS ||
          vkCreateSemaphore(deviceManager->getLogicalDevice(), &semaphoreInfo,
                            nullptr, &renderFinishedSemaphores[i]) ||
          vkCreateFence(deviceManager->getLogicalDevice(), &fenceInfo, nullptr,
                        &inFlightFences[i]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create semaphores!");
      }
    }
  }

  void createTextureImage() {
    textureImage = std::make_unique<TextureImage>(*deviceManager, *commandPool,
                                                  TEXTURE_PATH);
  }

  void createTextureImageView() {
    textureImageView = swapChain->createImageView(
        deviceManager->getLogicalDevice(), textureImage->getTextureImage(),
        VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT,
        textureImage->getMipLevels());
  }

  void createTextureSampler() {
    textureSampler = std::make_unique<TextureSampler>(
        *deviceManager, textureImage->getMipLevels());
  }

  void createVertexBuffer() {
    vertexBuffer =
        std::make_unique<VertexBuffer>(*deviceManager, *commandPool, vertices);
  }

  // TODO Look into packing vertex data and vertex indices into one
  // VkBuffer object using offsets. This way the data is more cache coherent
  // and the driver can optimize better. Look into 'aliasing'.
  void createIndexBuffer() {
    indexBuffer =
        std::make_unique<IndexBuffer>(*deviceManager, *commandPool, indices);
  }

  void createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    // Our strategy is to create buffers for each swapchain image
    auto swapChainImageSize = swapChain->getImageSize();
    uniformBuffers.resize(swapChainImageSize);

    for (size_t i = 0; i < swapChainImageSize; i++) {
      UniformBuffer uniformBuffer(*deviceManager, swapChainImageSize,
                                  bufferSize);
      uniformBuffers.push_back(uniformBuffer);
    }
  }

  // TODO Read up on what 'Push Constants' are. These are more efficient
  // compared to the current way of allocating UBOs
  void updateUniformBuffer(uint32_t currentImage) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     currentTime - startTime)
                     .count();

    auto swapChainExtent = swapChain->getExtent();
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view =
        glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(
        glm::radians(45.0f),
        swapChainExtent.width / static_cast<float>(swapChainExtent.height),
        0.1f, 10.0f);

    // Flip the y-axis of the projection matrix since OpenGL has inverted axis
    ubo.proj[1][1] *= -1;

    void* data;
    vkMapMemory(deviceManager->getLogicalDevice(),
                uniformBuffers[currentImage].getDeviceMemory(), 0, sizeof(ubo),
                0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(deviceManager->getLogicalDevice(),
                  uniformBuffers[currentImage].getDeviceMemory());
  }

  void createDescriptorPool() {
    descriptorPool = std::make_unique<DescriptorPool>(*deviceManager, *swapChain);
  }

  void createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(
        swapChain->getImageSize(),
        *descriptorSetLayout->getDescriptorSetLayout());
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool->getDescriptorPool();
    allocInfo.descriptorSetCount =
        static_cast<uint32_t>(swapChain->getImageSize());
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(swapChain->getImageSize());
    if (vkAllocateDescriptorSets(deviceManager->getLogicalDevice(), &allocInfo,
                                 descriptorSets.data()) != VK_SUCCESS) {
      throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < swapChain->getImageSize(); i++) {
      VkDescriptorBufferInfo bufferInfo = {};
      bufferInfo.buffer = uniformBuffers[i].getBuffer();
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(UniformBufferObject);

      VkDescriptorImageInfo imageInfo = {};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = textureImageView;
      imageInfo.sampler = textureSampler;

      std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = descriptorSets[i];
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &bufferInfo;

      descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[1].dstSet = descriptorSets[i];
      descriptorWrites[1].dstBinding = 1;
      descriptorWrites[1].dstArrayElement = 0;
      descriptorWrites[1].descriptorType =
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptorWrites[1].descriptorCount = 1;
      descriptorWrites[1].pImageInfo = &imageInfo;

      vkUpdateDescriptorSets(deviceManager->getLogicalDevice(),
                             static_cast<uint32_t>(descriptorWrites.size()),
                             descriptorWrites.data(), 0, nullptr);
    }
  }

  void createSurface() {
    if (glfwCreateWindowSurface(instance->getInstance(), window, nullptr,
                                &surface) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create window surface!");
    }
  }

  void createDeviceManager() {
    deviceManager = std::make_unique<DeviceManager>(*instance, surface,
                                                    enableValidationLayers);
  }

  void createFrameBuffers() {
    swapChain->createFrameBuffers(deviceManager->getLogicalDevice(),
                                  *renderPass, depthImageView);
  }

  void recreateSwapChain() {
    // This is done in case the window is minimized too small
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
      glfwGetFramebufferSize(window, &width, &height);
      glfwWaitEvents();
    }

    vkDeviceWaitIdle(deviceManager->getLogicalDevice());

    cleanupSwapChain();

    createSwapChain();
    createRenderPass();
    createGraphicsPipeline();
    createDepthResources();
    createFrameBuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
  }

  void createSwapChain() {
    // This also constructs the necessary VkImageViews
    swapChain = std::make_unique<Swapchain>(
        deviceManager->findQueueFamilies(surface),
        deviceManager->getSwapChainSupport(), deviceManager->getLogicalDevice(),
        surface, window);
  }

  void createRenderPass() {
    renderPass = std::make_unique<RenderPass>(deviceManager->getLogicalDevice(),
                                              swapChain->getImageFormat(),
                                              findDepthFormat());
  }

  void createGraphicsPipeline() {
    graphicsPipeline = std::make_unique<Pipeline>(
        deviceManager->getLogicalDevice(), swapChain, renderPass);
  }

  void loadModel() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                          MODEL_PATH.c_str())) {
      throw std::runtime_error(warn + err);
    }

    // For efficiency we only want unique vertices
    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
    for (const auto& shape : shapes) {
      for (const auto& index : shape.mesh.indices) {
        Vertex vertex = {};
        vertex.pos = {attrib.vertices[3 * index.vertex_index + 0],
                      attrib.vertices[3 * index.vertex_index + 1],
                      attrib.vertices[3 * index.vertex_index + 2]};

        vertex.texCoord = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            // Need to flip the coordinates to be compatible with Vulkan
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

        vertex.color = {1.0f, 1.0f, 1.0f};

        if (uniqueVertices.count(vertex) == 0) {
          uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
          vertices.push_back(vertex);
        }

        indices.push_back(uniqueVertices[vertex]);
      }
    }
  }

  static void framebufferResizeCallback(GLFWwindow* window, int width,
                                        int height) {
    auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
  }
};
}  // namespace odin

int main() {
  odin::App app;

  try {
    app.run();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}