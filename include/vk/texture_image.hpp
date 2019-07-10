#ifndef ODIN_TEXTURE_IMAGE_HPP
#define ODIN_TEXTURE_IMAGE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include "vk/buffer.hpp"
#include "vk/command_pool.hpp"
#include "vk/device_manager.hpp"

namespace odin {
class TextureImage {
 public:
  TextureImage(const DeviceManager& deviceManager,
               const CommandPool& commandPool, const std::string& texturePath);

  ~TextureImage();

  const uint32_t getMipLevels() const;

  const VkImage getTextureImage() const;

 private:
  void copyBufferToImage(const DeviceManager& deviceManager,
                         const CommandPool& commandPool, VkBuffer buffer,
                         VkImage image, uint32_t width, uint32_t height);

  void createImage(const DeviceManager& deviceManager, uint32_t width,
                   uint32_t height, uint32_t mipLevels, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties, VkImage& image,
                   VkDeviceMemory& imageMemory);

  void createTextureImage(const DeviceManager& deviceManager,
                          const CommandPool& commandPool,
                          const std::string& texturePath);

  void generateMipmaps(const DeviceManager& deviceManager,
                       const CommandPool& commandPool, VkImage image,
                       VkFormat imageFormat, int32_t texWidth,
                       int32_t texHeight, uint32_t mipLevels);

  bool hasStencilComponent(const VkFormat& format);

  void transitionImageLayout(const DeviceManager& deviceManager,
                             const CommandPool& commandPool, VkImage image,
                             VkFormat format, VkImageLayout oldLayout,
                             VkImageLayout newLayout, uint32_t mipLevels);

  uint32_t mipLevels;
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
};
}  // namespace odin
#endif  // ODIN_TEXTURE_IMAGE_HPP