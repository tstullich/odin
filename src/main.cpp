#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE  // Will need this because OpenGL uses \
                                    // [-1.0, 1.0] z-depth range
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>

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

#include "renderer/ubo.hpp"
#include "renderer/vertex.hpp"
#include "vk/command_pool.hpp"
#include "vk/depth_image.hpp"
#include "vk/descriptor_pool.hpp"
#include "vk/descriptor_set_layout.hpp"
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

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

namespace odin {

// rename boost namespace for readability
namespace po = boost::program_options;

class App {
 public:
  App(int argc, char* argv[]) {
    if (parseArguments(argc, argv)) {
      throw std::runtime_error("Unable to parse command line arguments!");
    }
  }

  void run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

 private:
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
    createTextureSampler();
    loadModel();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
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

  int parseArguments(int argc, char* argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()("help", "Produce help message")(
        "vs", po::value<std::string>(&VERTEX_SHADER_PATH),
        "Vertex shader path (SPIR-V)")(
        "fs", po::value<std::string>(&FRAGMENT_SHADER_PATH),
        "Fragment shader path (SPIR-V)")(
        "obj", po::value<std::string>(&MODEL_PATH), "OBJ model file path")(
        "tex", po::value<std::string>(&TEXTURE_PATH), "Texture file path");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 1;
    }

    if (vm.count("vs")) {
      std::cout << "Vertex shader path: " << VERTEX_SHADER_PATH << std::endl;
    } else {
      std::cout
          << "Vertex shader was not specified! Please specify the file path"
          << std::endl;
      std::cout << desc << std::endl;
      return 1;
    }

    if (vm.count("fs")) {
      std::cout << "Fragment shader path: " << FRAGMENT_SHADER_PATH
                << std::endl;
    } else {
      std::cout
          << "Fragment shader was not specified! Please specify the file path"
          << std::endl;
      return 1;
    }

    if (vm.count("obj")) {
      std::cout << "OBJ model path: " << MODEL_PATH << std::endl;
    } else {
      std::cout
          << "OBJ model file was not specified! Please specify the file path"
          << std::endl;
      return 1;
    }

    if (vm.count("tex")) {
      std::cout << "Texture file path: " << TEXTURE_PATH << std::endl;
    } else {
      std::cout << "Texture file path was not specified! Please specify the path" << std::endl;
      return 1;
    }

    return 0;
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
    vkDestroyImageView(deviceManager->getLogicalDevice(),
                       depthImage->getImageView(), nullptr);
    vkDestroyImage(deviceManager->getLogicalDevice(), depthImage->getImage(),
                   nullptr);
    vkFreeMemory(deviceManager->getLogicalDevice(),
                 depthImage->getDeviceMemory(), nullptr);

    for (auto framebuffer : swapChain->getFramebuffers()) {
      vkDestroyFramebuffer(deviceManager->getLogicalDevice(), framebuffer,
                           nullptr);
    }

    auto commandBuffers = commandPool->getCommandBuffers();
    vkFreeCommandBuffers(
        deviceManager->getLogicalDevice(), commandPool->getCommandPool(),
        static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    vkDestroyPipeline(deviceManager->getLogicalDevice(),
                      graphicsPipeline->getGraphicsPipeline(), nullptr);
    vkDestroyPipelineLayout(deviceManager->getLogicalDevice(),
                            graphicsPipeline->getPipelineLayout(), nullptr);
    vkDestroyRenderPass(deviceManager->getLogicalDevice(),
                        renderPass->getRenderPass(), nullptr);

    for (auto imageView : swapChain->getImageViews()) {
      vkDestroyImageView(deviceManager->getLogicalDevice(), imageView, nullptr);
    }

    vkDestroySwapchainKHR(deviceManager->getLogicalDevice(),
                          swapChain->getSwapchain(), nullptr);

    for (size_t i = 0; i < swapChain->getImageSize(); i++) {
      vkDestroyBuffer(deviceManager->getLogicalDevice(),
                      uniformBuffers[i].getBuffer(), nullptr);
      vkFreeMemory(deviceManager->getLogicalDevice(),
                   uniformBuffers[i].getDeviceMemory(), nullptr);
    }

    vkDestroyDescriptorPool(deviceManager->getLogicalDevice(),
                            descriptorPool->getDescriptorPool(), nullptr);
  }

  void cleanup() {
    cleanupSwapChain();

    vkDestroySampler(deviceManager->getLogicalDevice(),
                     textureSampler->getSampler(), nullptr);
    vkDestroyImageView(deviceManager->getLogicalDevice(),
                       textureImage->getTextureImageView(), nullptr);

    vkDestroyImage(deviceManager->getLogicalDevice(),
                   textureImage->getTextureImage(), nullptr);
    vkFreeMemory(deviceManager->getLogicalDevice(),
                 textureImage->getTextureImageMemory(), nullptr);

    vkDestroyDescriptorSetLayout(deviceManager->getLogicalDevice(),
                                 *descriptorSetLayout->getDescriptorSetLayout(),
                                 nullptr);

    vkDestroyBuffer(deviceManager->getLogicalDevice(), indexBuffer->getBuffer(),
                    nullptr);
    vkFreeMemory(deviceManager->getLogicalDevice(),
                 indexBuffer->getBufferMemory(), nullptr);

    vkDestroyBuffer(deviceManager->getLogicalDevice(),
                    vertexBuffer->getBuffer(), nullptr);
    vkFreeMemory(deviceManager->getLogicalDevice(),
                 vertexBuffer->getBufferMemory(), nullptr);

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
                                      *swapChain, *indexBuffer, *vertexBuffer,
                                      descriptorPool->getDescriptorSets());
  }

  void createCommandPool() {
    commandPool = std::make_unique<CommandPool>(
        deviceManager->getLogicalDevice(),
        deviceManager->findQueueFamilies(surface));
  }

  void createDescriptorSetLayout() {
    descriptorSetLayout = std::make_unique<DescriptorSetLayout>(*deviceManager);
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
                                                  *swapChain, TEXTURE_PATH);
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
    size_t swapChainImageSize = swapChain->getImageSize();
    // TODO See why this does not compile
    uniformBuffers.resize(swapChainImageSize);

    for (size_t i = 0; i < swapChainImageSize; i++) {
      uniformBuffers[i] =
          UniformBuffer(*deviceManager, swapChainImageSize, bufferSize);
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
    UniformBufferObject ubo;
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

  void createDepthResources() {
    depthImage =
        std::make_unique<DepthImage>(*deviceManager, *commandPool, *swapChain);
  }

  void createDescriptorPool() {
    // This also creates the necessary VkDescriptorSets
    descriptorPool = std::make_unique<DescriptorPool>(
        *deviceManager, *swapChain, *descriptorSetLayout, uniformBuffers,
        *textureImage, *textureSampler);
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
                                  *renderPass, depthImage->getImageView());
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
    renderPass = std::make_unique<RenderPass>(
        deviceManager->getLogicalDevice(), swapChain->getImageFormat(),
        DepthImage::findDepthFormat(*deviceManager));
  }

  void createGraphicsPipeline() {
    graphicsPipeline = std::make_unique<Pipeline>(
        deviceManager->getLogicalDevice(), *swapChain, *renderPass,
        *descriptorSetLayout);
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

  GLFWwindow* window;

  std::unique_ptr<odin::Instance> instance;
  VkSurfaceKHR surface;

  std::unique_ptr<DeviceManager> deviceManager;

  std::unique_ptr<Swapchain> swapChain;

  std::unique_ptr<RenderPass> renderPass;

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

  std::unique_ptr<DescriptorSetLayout> descriptorSetLayout;
  std::unique_ptr<DescriptorPool> descriptorPool;

  std::unique_ptr<TextureImage> textureImage;
  std::unique_ptr<TextureSampler> textureSampler;

  std::unique_ptr<DepthImage> depthImage;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  const int WIDTH = 800;
  const int HEIGHT = 600;
  const int MAX_FRAMES_IN_FLIGHT = 2;

  static std::string FRAGMENT_SHADER_PATH;
  static std::string VERTEX_SHADER_PATH;
  static std::string MODEL_PATH;
  static std::string TEXTURE_PATH;
};
}  // namespace odin

int main(int argc, char* argv[]) {
  try {
    odin::App app(argc, argv);
    app.run();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}