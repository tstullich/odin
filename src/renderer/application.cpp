#define TINYOBJLOADER_IMPLEMENTATION
#include "renderer/application.hpp"
#include <tiny_obj_loader.h>

// Rename boost namespace for readability
namespace po = boost::program_options;

// Exposing static members for usage in other classes
std::string odin::Application::COMPUTE_SHADER_PATH;
std::string odin::Application::FRAGMENT_SHADER_PATH;
std::string odin::Application::VERTEX_SHADER_PATH;
std::string odin::Application::MODEL_PATH;
std::string odin::Application::TEXTURE_PATH;

odin::Application::Application(int argc, char* argv[]) {
  if (parseArguments(argc, argv)) {
    throw std::runtime_error("Unable to parse command line arguments!");
  }
}

void odin::Application::framebufferResizeCallback(GLFWwindow* window, int width,
                                                  int height) {
  auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
  app->framebufferResized = true;
}

void odin::Application::cleanup() {
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

  vkDestroyBuffer(deviceManager->getLogicalDevice(), vertexBuffer->getBuffer(),
                  nullptr);
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

void odin::Application::cleanupSwapChain() {
  vkDestroyImageView(deviceManager->getLogicalDevice(),
                     depthImage->getImageView(), nullptr);
  vkDestroyImage(deviceManager->getLogicalDevice(), depthImage->getImage(),
                 nullptr);
  vkFreeMemory(deviceManager->getLogicalDevice(), depthImage->getDeviceMemory(),
               nullptr);

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

void odin::Application::createCommandBuffers() {
  commandPool->createCommandBuffers(deviceManager->getLogicalDevice(),
                                    *renderPass, *graphicsPipeline, *swapChain,
                                    *indexBuffer, *vertexBuffer,
                                    descriptorPool->getDescriptorSets());
}

void odin::Application::createCommandPool() {
  commandPool =
      std::make_unique<CommandPool>(deviceManager->getLogicalDevice(),
                                    deviceManager->findQueueFamilies(surface));
}

void odin::Application::createDepthResources() {
  depthImage =
      std::make_unique<DepthImage>(*deviceManager, *commandPool, *swapChain);
}

void odin::Application::createDescriptorPool() {
  // This also creates the necessary VkDescriptorSets
  descriptorPool = std::make_unique<DescriptorPool>(
      *deviceManager, *swapChain, *descriptorSetLayout, uniformBuffers,
      *textureImage, *textureSampler);
}

void odin::Application::createDescriptorSetLayout() {
  descriptorSetLayout = std::make_unique<DescriptorSetLayout>(*deviceManager);
}

void odin::Application::createDeviceManager() {
  deviceManager = std::make_unique<DeviceManager>(*instance, surface,
                                                  enableValidationLayers);
}

void odin::Application::createFrameBuffers() {
  swapChain->createFrameBuffers(deviceManager->getLogicalDevice(), *renderPass,
                                depthImage->getImageView());
}

void odin::Application::createGraphicsPipeline() {
  graphicsPipeline = std::make_unique<GraphicsPipeline>(
      deviceManager->getLogicalDevice(), *swapChain, *renderPass,
      *descriptorSetLayout);
}

// TODO Look into packing vertex data and vertex indices into one
// VkBuffer object using offsets. This way the data is more cache coherent
// and the driver can optimize better. Look into 'aliasing'.
void odin::Application::createIndexBuffer() {
  indexBuffer =
      std::make_unique<IndexBuffer>(*deviceManager, *commandPool, indices);
}

void odin::Application::createInstance() {
  instance = std::make_unique<odin::Instance>(enableValidationLayers);
}

void odin::Application::createRenderPass() {
  renderPass = std::make_unique<RenderPass>(
      deviceManager->getLogicalDevice(), swapChain->getImageFormat(),
      DepthImage::findDepthFormat(*deviceManager));
}

void odin::Application::createSurface() {
  if (glfwCreateWindowSurface(instance->getInstance(), window, nullptr,
                              &surface) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create window surface!");
  }
}

void odin::Application::createSwapChain() {
  // This also constructs the necessary VkImageViews
  swapChain = std::make_unique<Swapchain>(
      deviceManager->findQueueFamilies(surface),
      deviceManager->getSwapChainSupport(), deviceManager->getLogicalDevice(),
      surface, window);
}

void odin::Application::createSyncObjects() {
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

void odin::Application::createTextureImage() {
  textureImage = std::make_unique<TextureImage>(*deviceManager, *commandPool,
                                                *swapChain, TEXTURE_PATH);
}

void odin::Application::createTextureSampler() {
  textureSampler = std::make_unique<TextureSampler>(
      *deviceManager, textureImage->getMipLevels());
}

void odin::Application::createUniformBuffers() {
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

void odin::Application::createVertexBuffer() {
  vertexBuffer =
      std::make_unique<VertexBuffer>(*deviceManager, *commandPool, vertices);
}

void odin::Application::drawFrame() {
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

void odin::Application::initVulkan() {
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

void odin::Application::initWindow() {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "Odin", nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void odin::Application::loadModel() {
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

void odin::Application::mainLoop() {
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    drawFrame();
  }

  // Needed so that we do not destroy resources while
  // draw calls may still be going on
  vkDeviceWaitIdle(deviceManager->getLogicalDevice());
}

int odin::Application::parseArguments(int argc, char* argv[]) {
  po::options_description desc("Allowed options");
  desc.add_options()("help", "Produce help message")(
      "demo", "Runs renderer with pre-defined values")(
      "cs", po::value<std::string>(&COMPUTE_SHADER_PATH),
      "Compute shader path (SPIR-V)")(
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

  // Check if we have enabled demo mode
  if (vm.count("demo")) {
    std::cout << "Running in demo mode" << std::endl;
    COMPUTE_SHADER_PATH = "shaders/comp.spv";
    FRAGMENT_SHADER_PATH = "shaders/frag.spv";
    VERTEX_SHADER_PATH = "shaders/vert.spv";
    MODEL_PATH = "models/triangle.obj";
    TEXTURE_PATH = "textures/texture.jpg";
    return 0;
  }

  if (vm.count("cs")) {
    std::cout << "Compute shader path: " << COMPUTE_SHADER_PATH << std::endl;
  } else {
    std::cout
        << "Compute shader was not specified! Please specify the file path"
        << std::endl;
    std::cout << desc << std::endl;
    return 1;
  }

  if (vm.count("vs")) {
    std::cout << "Vertex shader path: " << VERTEX_SHADER_PATH << std::endl;
  } else {
    std::cout << "Vertex shader was not specified! Please specify the file path"
              << std::endl;
    std::cout << desc << std::endl;
    return 1;
  }

  if (vm.count("fs")) {
    std::cout << "Fragment shader path: " << FRAGMENT_SHADER_PATH << std::endl;
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
    std::cout << "Texture file path was not specified! Please specify the path"
              << std::endl;
    return 1;
  }

  return 0;
}

void odin::Application::recreateSwapChain() {
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

void odin::Application::run() {
  initWindow();
  initVulkan();
  mainLoop();
  cleanup();
}

// TODO Read up on what 'Push Constants' are. These are more efficient
// compared to the current way of allocating UBOs
void odin::Application::updateUniformBuffer(uint32_t currentImage) {
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
      swapChainExtent.width / static_cast<float>(swapChainExtent.height), 0.1f,
      10.0f);

  // Flip the y-axis of the projection matrix since OpenGL has inverted axis
  ubo.proj[1][1] *= -1;

  void* data;
  vkMapMemory(deviceManager->getLogicalDevice(),
              uniformBuffers[currentImage].getDeviceMemory(), 0, sizeof(ubo), 0,
              &data);
  memcpy(data, &ubo, sizeof(ubo));
  vkUnmapMemory(deviceManager->getLogicalDevice(),
                uniformBuffers[currentImage].getDeviceMemory());
}
