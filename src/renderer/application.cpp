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
const int odin::Application::WIDTH;
const int odin::Application::HEIGHT;
odin::Camera odin::Application::camera;

odin::Application::Application(int argc, char *argv[]) {
  if (parseArguments(argc, argv)) {
    throw std::runtime_error("Unable to parse command line arguments!");
  }
}

void odin::Application::framebufferResizeCallback(GLFWwindow *window, int width,
                                                  int height) {
  auto app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
  app->framebufferResized = true;
}

void odin::Application::keyCallback(GLFWwindow *window, int key, int scanCode,
                                    int action, int mods) {
  bool keyPressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
  // Move the camera based on keystrokes
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  } else if (key == GLFW_KEY_W && keyPressed) {
    camera.origin.z -= 0.1;
  } else if (key == GLFW_KEY_A && keyPressed) {
    camera.origin.x -= 0.1;
  } else if (key == GLFW_KEY_S && keyPressed) {
    camera.origin.z += 0.1;
  } else if (key == GLFW_KEY_D && keyPressed) {
    camera.origin.x += 0.1;
  }
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

  vkDestroyDescriptorSetLayout(
      deviceManager->getLogicalDevice(),
      *graphicsDescriptorSetLayout->getDescriptorSetLayout(), nullptr);

  vkDestroyDescriptorSetLayout(
      deviceManager->getLogicalDevice(),
      *computeDescriptorSetLayout->getDescriptorSetLayout(), nullptr);

  vkDestroyBuffer(deviceManager->getLogicalDevice(), bvhBuffer->getBuffer(),
                  nullptr);
  vkFreeMemory(deviceManager->getLogicalDevice(), bvhBuffer->getBufferMemory(),
               nullptr);

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(deviceManager->getLogicalDevice(),
                       imageAvailableSemaphores[i], nullptr);
    vkDestroySemaphore(deviceManager->getLogicalDevice(),
                       renderFinishedSemaphores[i], nullptr);
    vkDestroyFence(deviceManager->getLogicalDevice(), inFlightFences[i],
                   nullptr);
  }

  vkDestroyCommandPool(deviceManager->getLogicalDevice(),
                       commandPool->getComputeCommandPool(), nullptr);

  vkDestroyCommandPool(deviceManager->getLogicalDevice(),
                       commandPool->getGraphicsCommandPool(), nullptr);

  vkDestroyDevice(deviceManager->getLogicalDevice(), nullptr);

  vkDestroySurfaceKHR(instance->getInstance(), surface, nullptr);

  glfwDestroyWindow(window);

  glfwTerminate();
}

void odin::Application::cleanupSwapChain() {
  for (auto framebuffer : swapChain->getFramebuffers()) {
    vkDestroyFramebuffer(deviceManager->getLogicalDevice(), framebuffer,
                         nullptr);
  }

  auto commandBuffers = commandPool->getGraphicsCommandBuffers();
  vkFreeCommandBuffers(
      deviceManager->getLogicalDevice(), commandPool->getGraphicsCommandPool(),
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

  vkDestroyBuffer(deviceManager->getLogicalDevice(), computeUbo->getBuffer(),
                  nullptr);

  vkFreeMemory(deviceManager->getLogicalDevice(), computeUbo->getDeviceMemory(),
               nullptr);

  vkDestroyDescriptorPool(deviceManager->getLogicalDevice(),
                          descriptorPool->getDescriptorPool(), nullptr);
}

void odin::Application::createBvh() {
  std::cout << "Building BVH" << std::endl;
  bvh.init(triangles);
  for (auto &n : bvh.nodes) {
    std::cout << "Left Node" << std::endl;
    std::cout << "V1: " << n.left.v0.x << ", " << n.left.v0.y << ", " << n.left.v0.z << std::endl;
    std::cout << "V2: " << n.left.v1.x << ", " << n.left.v1.y << ", " << n.left.v1.z << std::endl;
    std::cout << "V3: " << n.left.v2.x << ", " << n.left.v2.y << ", " << n.left.v2.z << std::endl;
    std::cout << "Right Node" << std::endl;
    std::cout << "V1: " << n.right.v0.x << ", " << n.right.v0.y << ", " << n.right.v0.z << std::endl;
    std::cout << "V2: " << n.right.v1.x << ", " << n.right.v1.y << ", " << n.right.v1.z << std::endl;
    std::cout << "V3: " << n.right.v2.x << ", " << n.right.v2.y << ", " << n.right.v2.z << std::endl;
    std::cout << "AABB" << std::endl;
    std::cout << "Min: " << n.box.min.x << ", " << n.box.min.y << ", " << n.box.min.z << std::endl;
    std::cout << "Max: " << n.box.max.x << ", " << n.box.max.y << ", " << n.box.max.z << std::endl;
    std::cout << std::endl;
  }
  std::cout << "Finished building BVH" << std::endl;
}

void odin::Application::createBvhBuffer() {
  bvhBuffer =
      std::make_unique<BvhBuffer>(*deviceManager, *commandPool, bvh.nodes);
}

void odin::Application::createCommandBuffers() {
  commandPool->createComputeCommandBuffers(
      deviceManager->getLogicalDevice(), *renderPass, *computePipeline,
      *descriptorPool, textureImage->getWidth(), textureImage->getHeight());

  commandPool->createGraphicsCommandBuffers(
      deviceManager->getLogicalDevice(), *renderPass, *graphicsPipeline,
      *descriptorPool, *swapChain, *textureImage);
}

void odin::Application::createCommandPool() {
  commandPool =
      std::make_unique<CommandPool>(deviceManager->getLogicalDevice(),
                                    deviceManager->findQueueFamilies(surface));
}

void odin::Application::createComputePipeline() {
  computePipeline = std::make_unique<ComputePipeline>(
      *deviceManager, *computeDescriptorSetLayout, COMPUTE_SHADER_PATH);
}

void odin::Application::createDepthResources() {
  depthImage =
      std::make_unique<DepthImage>(*deviceManager, *commandPool, *swapChain);
}

void odin::Application::createDescriptorPool() {
  // Grab all of the needed descriptors for binding
  std::vector<VkDescriptorBufferInfo> bufferInfos;
  bufferInfos.push_back(computeUbo->getDescriptor());
  bufferInfos.push_back(bvhBuffer->getDescriptor());

  // This also creates the necessary VkDescriptorSets
  descriptorPool = std::make_unique<DescriptorPool>(
      *deviceManager, *swapChain, *computeDescriptorSetLayout,
      *graphicsDescriptorSetLayout, *textureImage, *textureSampler,
      bufferInfos);
}

void odin::Application::createDescriptorSetLayouts() {
  computeDescriptorSetLayout =
      std::make_unique<DescriptorSetLayout>(*deviceManager, true);

  graphicsDescriptorSetLayout =
      std::make_unique<DescriptorSetLayout>(*deviceManager, false);
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
      *graphicsDescriptorSetLayout, VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);
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

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
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

  if (vkCreateFence(deviceManager->getLogicalDevice(), &fenceInfo, nullptr,
                    &computeFence) != VK_SUCCESS) {
    throw std::runtime_error("Unable to create fence for compute pipeline!");
  }
}

void odin::Application::createTextureImage() {
  textureImage = std::make_unique<TextureImage>(
      *deviceManager, *commandPool, *swapChain, *textureSampler, WIDTH, HEIGHT);
}

void odin::Application::createTextureSampler() {
  textureSampler = std::make_unique<TextureSampler>(*deviceManager);
}

void odin::Application::createUniformBuffers() {
  // Initialize camera
  glm::vec3 lookFrom(0.0f, 0.0f, 6.0f);
  glm::vec3 lookAt(0.0f, 0.0f, -1.0f);
  float distToFocus = glm::length(lookFrom - lookAt);
  float aperture = 2.0;

  camera.init(lookFrom, lookAt, glm::vec3(0.0f, 1.0f, 0.0f), 20,
              static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), aperture,
              distToFocus);

  // Create a UBO to pass various information to the compute shader
  VkDeviceSize bufferSize = sizeof(Camera);
  computeUbo = std::make_unique<UniformBuffer>(*deviceManager, bufferSize);

  // Copy camera data into memory
  void *data;
  vkMapMemory(deviceManager->getLogicalDevice(), computeUbo->getDeviceMemory(),
              0, sizeof(Camera), 0, &data);
  memcpy(data, &camera, sizeof(Camera));
  vkUnmapMemory(deviceManager->getLogicalDevice(),
                computeUbo->getDeviceMemory());
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
  submitInfo.pCommandBuffers =
      commandPool->getGraphicsCommandBuffer(imageIndex);

  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  vkResetFences(deviceManager->getLogicalDevice(), 1,
                &inFlightFences[currentFrame]);

  if (vkQueueSubmit(deviceManager->getGraphicsQueue(), 1, &submitInfo,
                    inFlightFences[currentFrame]) != VK_SUCCESS) {
    throw std::runtime_error("Failed to submit draw command buffer!");
  }

  VkPresentInfoKHR graphicsPresentInfo = {};
  graphicsPresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  graphicsPresentInfo.waitSemaphoreCount = 1;
  graphicsPresentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swapChain->getSwapchain()};
  graphicsPresentInfo.swapchainCount = 1;
  graphicsPresentInfo.pSwapchains = swapChains;
  graphicsPresentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(deviceManager->getPresentationQueue(),
                             &graphicsPresentInfo);

  // This check should be done after vkQueuePresentKHR to avoid
  // improperly signalled Semaphores
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      framebufferResized) {
    framebufferResized = false;
    recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to present swap chain image!");
  }

  // Wait for compute buffer to before submitting again
  vkWaitForFences(deviceManager->getLogicalDevice(), 1, &computeFence, VK_TRUE,
                  UINT64_MAX);
  vkResetFences(deviceManager->getLogicalDevice(), 1, &computeFence);

  VkSubmitInfo computeSubmitInfo = {};
  computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  computeSubmitInfo.commandBufferCount = 1;
  computeSubmitInfo.pCommandBuffers = commandPool->getComputeCommandBuffer();

  if (vkQueueSubmit(deviceManager->getComputeQueue(), 1, &computeSubmitInfo,
                    computeFence) != VK_SUCCESS) {
    throw std::runtime_error("Unable to submit to compute queue!");
  }

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void odin::Application::initVulkan() {
  createInstance();
  createSurface();
  createDeviceManager();
  createUniformBuffers();
  loadModel();
  createBvh();
  createDescriptorSetLayouts();
  createSwapChain();
  createRenderPass();
  createCommandPool();
  createTextureSampler();
  createTextureImage();
  createGraphicsPipeline();
  createComputePipeline();
  createBvhBuffer();
  createDescriptorPool();
  createDepthResources();
  createFrameBuffers();
  createSyncObjects();
  createCommandBuffers();
}

void odin::Application::initWindow() {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "Odin", nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

  glfwSetKeyCallback(window, keyCallback);
}

void odin::Application::loadModel() {
  std::cout << "Loading model" << std::endl;
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
  for (auto const &shape : shapes) {
    size_t indexOffset = 0;
    for (auto const &faceIdx : shape.mesh.num_face_vertices) {
      Triangle triangle = {};
      auto idx = shape.mesh.indices[indexOffset];
      triangle.v0 = {attrib.vertices[3 * idx.vertex_index + 0],
                     attrib.vertices[3 * idx.vertex_index + 1],
                     attrib.vertices[3 * idx.vertex_index + 2]};

      idx = shape.mesh.indices[indexOffset + 1];
      triangle.v1 = {attrib.vertices[3 * idx.vertex_index + 0],
                     attrib.vertices[3 * idx.vertex_index + 1],
                     attrib.vertices[3 * idx.vertex_index + 2]};

      idx = shape.mesh.indices[indexOffset + 2];
      triangle.v2 = {attrib.vertices[3 * idx.vertex_index + 0],
                     attrib.vertices[3 * idx.vertex_index + 1],
                     attrib.vertices[3 * idx.vertex_index + 2]};

      triangle.calculateNormal();
      triangles.push_back(triangle);
      indexOffset += faceIdx;
    }
  }
  std::cout << "Finished loading models" << std::endl;
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

int odin::Application::parseArguments(int argc, char *argv[]) {
  po::options_description desc("Allowed options");
  desc.add_options()("help", "Produce help message")(
      "demo", "Runs renderer with pre-defined values")(
      "obj", po::value<std::string>(&MODEL_PATH), "OBJ model file path")(
      "tex", po::value<std::string>(&TEXTURE_PATH), "Texture file path");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  // Set these by default
  COMPUTE_SHADER_PATH = "shaders/comp.spv";
  FRAGMENT_SHADER_PATH = "shaders/frag.spv";
  VERTEX_SHADER_PATH = "shaders/vert.spv";

  // Check if we have enabled demo mode
  if (vm.count("demo")) {
    std::cout << "Running in demo mode" << std::endl;
    MODEL_PATH = "models/triangle.obj";
    TEXTURE_PATH = "textures/texture.jpg";
    return 0;
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
  // Copy camera data into memory
  void *data;
  vkMapMemory(deviceManager->getLogicalDevice(), computeUbo->getDeviceMemory(),
              0, sizeof(Camera), 0, &data);
  memcpy(data, &camera, sizeof(Camera));
  vkUnmapMemory(deviceManager->getLogicalDevice(),
                computeUbo->getDeviceMemory());
}