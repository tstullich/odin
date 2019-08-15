#ifndef ODIN_APPLICATION_HPP
#define ODIN_APPLICATION_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

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

#include "renderer/camera.hpp"
#include "renderer/ubo.hpp"
#include "renderer/triangle.hpp"
#include "renderer/vertex.hpp"
#include "vk/compute_pipeline.hpp"
#include "vk/depth_image.hpp"
#include "vk/descriptor_pool.hpp"
#include "vk/descriptor_set_layout.hpp"
#include "vk/device_manager.hpp"
#include "vk/index_buffer.hpp"
#include "vk/instance.hpp"
#include "vk/graphics_pipeline.hpp"
#include "vk/render_pass.hpp"
#include "vk/swapchain.hpp"
#include "vk/texture_image.hpp"
#include "vk/texture_sampler.hpp"
#include "vk/triangle_buffer.hpp"
#include "vk/uniform_buffer.hpp"
#include "vk/vertex_buffer.hpp"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

namespace odin {

// Rename boost namespace for readability
namespace po = boost::program_options;

class Application {
 public:
  Application(int argc, char* argv[]);

  void run();

  static std::string COMPUTE_SHADER_PATH;
  static std::string FRAGMENT_SHADER_PATH;
  static std::string VERTEX_SHADER_PATH;
  static std::string MODEL_PATH;
  static std::string TEXTURE_PATH;
  static const int WIDTH = 800;
  static const int HEIGHT = 600;

 private:
  static void framebufferResizeCallback(GLFWwindow* window, int width,
                                        int height);
  void cleanup();

  void cleanupSwapChain();

  void createCommandBuffers();

  void createCommandPool();

  void createComputePipeline();

  void createDepthResources();

  void createDescriptorPool();

  void createDescriptorSetLayouts();

  void createDeviceManager();

  void createFrameBuffers();

  void createGraphicsPipeline();

  void createInstance();

  void createRenderPass();

  void createSurface();

  void createSwapChain();

  void createSyncObjects();

  void createTextureImage();

  void createTextureSampler();

  void createTriangleBuffer();

  void createUniformBuffers();

  void drawFrame();

  void initVulkan();

  void initWindow();

  void loadModel();

  void mainLoop();

  int parseArguments(int argc, char* argv[]);

  void recreateSwapChain();

  void updateUniformBuffer(uint32_t currentImage);

  GLFWwindow* window;

  std::unique_ptr<odin::Instance> instance;
  VkSurfaceKHR surface;

  std::unique_ptr<DeviceManager> deviceManager;

  std::unique_ptr<Swapchain> swapChain;

  std::unique_ptr<RenderPass> renderPass;

  std::unique_ptr<ComputePipeline> computePipeline;
  std::unique_ptr<GraphicsPipeline> graphicsPipeline;

  std::unique_ptr<CommandPool> commandPool;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  VkFence computeFence;
  size_t currentFrame = 0;

  bool framebufferResized = false;

  std::vector<Triangle> triangles;
  std::unique_ptr<TriangleBuffer> triangleBuffer;

  std::unique_ptr<UniformBuffer> computeUbo;

  std::unique_ptr<DescriptorSetLayout> computeDescriptorSetLayout;
  std::unique_ptr<DescriptorSetLayout> graphicsDescriptorSetLayout;
  std::unique_ptr<DescriptorPool> descriptorPool;

  std::unique_ptr<TextureImage> textureImage;
  std::unique_ptr<TextureSampler> textureSampler;

  std::unique_ptr<DepthImage> depthImage;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  const int MAX_FRAMES_IN_FLIGHT = 2;
};
}  // namespace odin
#endif  // ODIN_APPLICATION_HPP