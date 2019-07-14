#ifndef ODIN_INDEX_BUFFER_HPP
#define ODIN_INDEX_BUFFER_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "vk/buffer.hpp"
#include "vk/device_manager.hpp"

namespace odin {

// forward declarations
class CommandPool;

class IndexBuffer : public Buffer {
 public:
  IndexBuffer(const DeviceManager& deviceManager,
              const CommandPool& commandPool,
              const std::vector<uint32_t>& indices);

  ~IndexBuffer();

  const VkBuffer getBuffer() const;

  const VkDeviceMemory getBufferMemory() const;

  const uint32_t getNumIndices() const;

 private:
  uint32_t numIndices;
  VkDeviceMemory indexBufferMemory;
};
}  // namespace odin
#endif  // ODIN_INDEX_BUFFER_HPP