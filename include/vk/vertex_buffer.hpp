
#ifndef ODIN_VERTEX_BUFFER_HPP
#define ODIN_VERTEX_BUFFER_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstring>
#include <iostream>
#include <vector>

#include "vk/buffer.hpp"

// TODO Look into packing vertex data and vertex indices into one
// VkBuffer object using offsets. This way the data is more cache coherent
// and the driver can optimize better. Look into 'aliasing'.
namespace odin {
class VertexBuffer : Buffer {
 public:
  VertexBuffer(const VkDevice& logicalDevice,
               const VkPhysicalDevice& physicalDevice,
               const std::vector<Vertex>& vertices);

  ~VertexBuffer();

 private:
  VkDeviceMemory vertexBufferMemory;
};
}  // namespace odin
#endif  // ODIN_VERTEX_BUFFER_HPP