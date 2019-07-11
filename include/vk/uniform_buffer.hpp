#ifndef ODIN_UNIFORM_BUFFER_HPP
#define ODIN_UNIFORM_BUFFER_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

#include "vk/buffer.hpp"

namespace odin {
class UniformBuffer : public Buffer {
 public:
  UniformBuffer(const DeviceManager& deviceManager,
                const size_t swapChainImageSize, const VkDeviceSize bufferSize);

  ~UniformBuffer();

  const VkBuffer getBuffer() const;

  const VkDeviceMemory getDeviceMemory() const;

 private:
  VkDeviceMemory uniformBufferMemory;
};
}  // namespace odin
#endif  // ODIN_UNIFORM_BUFFER_HPP