#ifndef ODIN_TEXTURE_HPP
#define ODIN_TEXTURE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>
#include <stdexcept>
#include <string>

namespace odin {
class Texture {
 public:
  Texture(const std::string& texturePath);

  ~Texture();

 private:
  uint32_t mipLevels;

  void createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                   VkFormat format, VkImageTiling tiling,
                   VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                   VkImage& image, VkDeviceMemory& imageMemory);

  void createTextureImage();
};
}  // namespace odin
#endif  // ODIN_TEXTURE_HPP