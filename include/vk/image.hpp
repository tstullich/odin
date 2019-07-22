#ifndef ODIN_IMAGE_HPP
#define ODIN_IMAGE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

#include "vk/buffer.hpp"
#include "vk/device_manager.hpp"

namespace odin {
class Image {
 public:
 protected:
  Image() {
    // Dummy constructor. This should not be called and
    // instead one of the subclasses should be used to
    // create image resources
  }

  void createImage(const DeviceManager& deviceManager, uint32_t width,
                   uint32_t height, VkFormat format, VkImageTiling tiling,
                   VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                   VkImage& image, VkDeviceMemory& imageMemory);

  bool hasStencilComponent(const VkFormat& format);

  void transitionImageLayout(const DeviceManager& deviceManager,
                             const CommandPool& commandPool, VkImage image,
                             VkFormat format, VkImageLayout oldLayout,
                             VkImageLayout newLayout);

  VkImage image;
};
}  // namespace odin
#endif  // ODIN_IMAGE_HPP