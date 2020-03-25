#ifndef ODIN_TEXTURE_IMAGE_HPP
#define ODIN_TEXTURE_IMAGE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include "vk/buffer.hpp"
#include "vk/command_pool.hpp"
#include "vk/device_manager.hpp"
#include "vk/image.hpp"
#include "vk/swapchain.hpp"
#include "vk/texture_sampler.hpp"

namespace odin {
class TextureImage : Image {
 public:
  TextureImage(const DeviceManager& deviceManager,
               const CommandPool& commandPool, const Swapchain& swapChain,
               const TextureSampler& textureSampler, uint32_t width,
               uint32_t height);

  const VkDescriptorImageInfo* getDescriptor() const;

  const uint32_t getHeight() const;

  const VkImage getTextureImage() const;

  const VkDeviceMemory getTextureImageMemory() const;

  const VkImageView getTextureImageView() const;

  const uint32_t getWidth() const;

 private:
  void copyBufferToImage(const DeviceManager& deviceManager,
                         const CommandPool& commandPool, VkBuffer buffer,
                         VkImage image, uint32_t width, uint32_t height);

  void createTextureImage(const DeviceManager& deviceManager,
                          const CommandPool& commandPool, uint32_t width,
                          uint32_t height);

  void createTextureImageView(const DeviceManager& deviceManager,
                              const Swapchain& swapChain);

  bool hasStencilComponent(const VkFormat& format);

  VkDescriptorImageInfo descriptor;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  uint32_t imageHeight;
  uint32_t imageWidth;
};
}  // namespace odin
#endif  // ODIN_TEXTURE_IMAGE_HPP