#ifndef ODIN_TEXTURE_IMAGE_HPP
#define ODIN_TEXTURE_IMAGE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include "renderer/application.hpp"
#include "vk/buffer.hpp"
#include "vk/command_pool.hpp"
#include "vk/device_manager.hpp"
#include "vk/image.hpp"

namespace odin {
class TextureImage : Image {
 public:
  TextureImage(const DeviceManager& deviceManager,
               const CommandPool& commandPool, const Swapchain& swapChain,
               const std::string& texturePath);

  ~TextureImage();

  const uint32_t getMipLevels() const;

  const VkImage getTextureImage() const;

  const VkDeviceMemory getTextureImageMemory() const;

  const VkImageView getTextureImageView() const;

 private:
  void copyBufferToImage(const DeviceManager& deviceManager,
                         const CommandPool& commandPool, VkBuffer buffer,
                         VkImage image, uint32_t width, uint32_t height);

  void createTextureImage(const DeviceManager& deviceManager,
                          const CommandPool& commandPool,
                          const std::string& texturePath);

  void createTextureImageView(const DeviceManager& deviceManager,
                              const Swapchain& swapChain);

  bool hasStencilComponent(const VkFormat& format);

  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
};
}  // namespace odin
#endif  // ODIN_TEXTURE_IMAGE_HPP