#ifndef ODIN_INDEX_BUFFER_HPP
#define ODIN_INDEX_BUFFER_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "vk/buffer.hpp"

namespace odin {
class IndexBuffer : Buffer {
 public:
  IndexBuffer(const VkDevice& logicalDevice,
              const VkPhysicalDevice& physicalDevice,
              const std::vector<uint32_t>& indices);

  ~IndexBuffer();

 private:
  VkDeviceMemory indexBufferMemory;
};
}  // namespace odin
#endif  // ODIN_INDEX_BUFFER_HPP