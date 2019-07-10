#ifndef ODIN_DEPTH_IMAGE_HPP
#define ODIN_DEPTH_IMAGE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>

#include "vk/command_pool.hpp"
#include "vk/image.hpp"
#include "vk/swapchain.hpp"

namespace odin {
class DepthImage : Image {
 public:
  DepthImage(const DeviceManager& deviceManager, const CommandPool& commandPool,
             const Swapchain& swapChain);

  ~DepthImage();

  const VkDeviceMemory getDeviceMemory() const;

  const VkImage getImage() const;

  const VkImageView getImageView() const;

  static VkFormat findDepthFormat(const DeviceManager& deviceManager);

 private:
  void createDepthResources(const DeviceManager& deviceManager,
                            const CommandPool& commandPool,
                            const Swapchain& swapChain);

  static VkFormat findSupportedFormat(const DeviceManager& deviceManager,
                                      const std::vector<VkFormat>& candidates,
                                      VkImageTiling tiling,
                                      VkFormatFeatureFlags features);

  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;
};
}  // namespace odin
#endif  // ODIN_DEPTH_IMAGE_HPP