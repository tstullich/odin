#include "vk/vertex_buffer.hpp"
#include "vk/command_pool.hpp"

odin::VertexBuffer::VertexBuffer(const DeviceManager& deviceManager,
                                 const CommandPool& commandPool,
                                 const std::vector<Vertex>& vertices) {
  // Map buffer to CPU memory
  numVertices = vertices.size();
  VkDeviceSize bufferSize = sizeof(vertices[0]) * numVertices;
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(deviceManager.getPhysicalDevice(),
               deviceManager.getLogicalDevice(), bufferSize,
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);

  // TODO Look into adding some validation logic before copying
  void* data;
  vkMapMemory(deviceManager.getLogicalDevice(), stagingBufferMemory, 0,
              bufferSize, 0, &data);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(deviceManager.getLogicalDevice(), stagingBufferMemory);

  createBuffer(deviceManager.getPhysicalDevice(),
               deviceManager.getLogicalDevice(), bufferSize,
               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                   VK_BUFFER_USAGE_TRANSFER_DST_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, vertexBufferMemory);

  copyBuffer(deviceManager, commandPool, stagingBuffer, buffer, bufferSize);

  // Destroy the staging buffer and backing memory once we successfully copied
  // our vertex data into GPU memory
  vkDestroyBuffer(deviceManager.getLogicalDevice(), stagingBuffer, nullptr);
  vkFreeMemory(deviceManager.getLogicalDevice(), stagingBufferMemory, nullptr);

  // Setup descriptor
  descriptor.offset = 0;
  descriptor.buffer = buffer;
  descriptor.range = VK_WHOLE_SIZE;
}

odin::VertexBuffer::~VertexBuffer() {
  std::cout << "IMPLEMENT VERTEX BUFFER DESTRUCTOR!" << std::endl;
}

const VkBuffer odin::VertexBuffer::getBuffer() const { return buffer; }

const VkDeviceMemory odin::VertexBuffer::getBufferMemory() const {
  return vertexBufferMemory;
}

const VkDescriptorBufferInfo odin::VertexBuffer::getDescriptor() const {
  return descriptor;
}

const size_t odin::VertexBuffer::getVertexCount() const { return numVertices; }