#ifndef ODIN_TRIANGLE_BUFFER_HPP
#define ODIN_TRIANGLE_BUFFER_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include "renderer/triangle.hpp"
#include "vk/buffer.hpp"
#include "vk/device_manager.hpp"

namespace odin {

// Forward declarations
class CommandPool;

class TriangleBuffer : public Buffer {
 public:
  TriangleBuffer(const DeviceManager& deviceManager,
                 const CommandPool& commandPool,
                 const std::vector<Triangle>& triangles);
  
  ~TriangleBuffer();

  const VkBuffer getBuffer() const;

  const VkDeviceMemory getBufferMemory() const;

  const VkDescriptorBufferInfo getDescriptor() const;

  const size_t getTriangleCount() const;

 private:
  VkDeviceMemory triangleBufferMemory;
  size_t numTriangles;
};
}  // namespace odin
#endif  // ODIN_TRIANGLE_BUFFER_HPP