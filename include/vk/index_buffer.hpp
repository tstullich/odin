#ifndef ODIN_INDEX_BUFFER_HPP
#define ODIN_INDEX_BUFFER_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "vk/buffer.hpp"
#include "vk/command_pool.hpp"
#include "vk/device_manager.hpp"

namespace odin {
class IndexBuffer : Buffer {
 public:
  IndexBuffer(const DeviceManager& deviceManager,
              const CommandPool& commandPool,
              const std::vector<uint32_t>& indices);

  ~IndexBuffer();

  const VkBuffer getBuffer() const;

 private:
  VkDeviceMemory indexBufferMemory;
};
}  // namespace odin
#endif  // ODIN_INDEX_BUFFER_HPP