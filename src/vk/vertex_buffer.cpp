#include "vk/vertex_buffer.hpp"

odin::VertexBuffer::VertexBuffer(const VkDevice& logicalDevice,
                                 const VkPhysicalDevice& physicalDevice,
                                 const std::vector<Vertex>& vertices) {
  // Map buffer to CPU memory
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(physicalDevice, logicalDevice, bufferSize,
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);

  // TODO Look into adding some validation logic before copying
  void* data;
  vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(logicalDevice, stagingBufferMemory);

  createBuffer(
      physicalDevice, logicalDevice, bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, vertexBufferMemory);

  copyBuffer(stagingBuffer, buffer, bufferSize);

  // Destroy the staging buffer and backing memory once we successfully copied
  // our vertex data into GPU memory
  vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

odin::VertexBuffer::~VertexBuffer() {
  std::cout << "IMPLEMENT VERTEX BUFFER DESTRUCTOR!" << std::endl;
}