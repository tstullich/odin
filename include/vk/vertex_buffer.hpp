#ifndef ODIN_VERTEX_BUFFER_HPP
#define ODIN_VERTEX_BUFFER_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstring>
#include <iostream>
#include <vector>

#include "renderer/vertex.hpp"
#include "vk/buffer.hpp"
#include "vk/command_pool.hpp"
#include "vk/device_manager.hpp"

// TODO Look into packing vertex data and vertex indices into one
// VkBuffer object using offsets. This way the data is more cache coherent
// and the driver can optimize better. Look into 'aliasing'.
namespace odin {

class VertexBuffer : public Buffer {
 public:
  VertexBuffer(const DeviceManager& deviceManager,
               const CommandPool& commandPool,
               const std::vector<Vertex>& vertices);

  ~VertexBuffer();

  const VkBuffer getBuffer() const;

  const VkDeviceMemory getBufferMemory() const;

  const size_t getVertexCount() const;

 private:
  VkDeviceMemory vertexBufferMemory;
  size_t numVertices;
};
}  // namespace odin
#endif  // ODIN_VERTEX_BUFFER_HPP