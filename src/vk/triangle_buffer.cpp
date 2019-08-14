#include "vk/triangle_buffer.hpp"
#include "vk/command_pool.hpp"

odin::TriangleBuffer::TriangleBuffer(const DeviceManager& deviceManager,
                                     const CommandPool& commandPool,
                                     const std::vector<Triangle>& triangles) {
  numTriangles = triangles.size();
  VkDeviceSize bufferSize = sizeof(triangles[0]) * numTriangles;
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(deviceManager.getPhysicalDevice(),
               deviceManager.getLogicalDevice(), bufferSize,
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);

  void* data;
  vkMapMemory(deviceManager.getLogicalDevice(), stagingBufferMemory, 0,
              bufferSize, 0, &data);
  memcpy(data, triangles.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(deviceManager.getLogicalDevice(), stagingBufferMemory);

  createBuffer(
      deviceManager.getPhysicalDevice(), deviceManager.getLogicalDevice(),
      bufferSize,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
          VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, triangleBufferMemory);

  copyBuffer(deviceManager, commandPool, stagingBuffer, buffer, bufferSize);

  vkDestroyBuffer(deviceManager.getLogicalDevice(), stagingBuffer, nullptr);
  vkFreeMemory(deviceManager.getLogicalDevice(), stagingBufferMemory, nullptr);

  // Setup descriptor
  descriptor.offset = 0;
  descriptor.buffer = buffer;
  descriptor.range = VK_WHOLE_SIZE;
}

odin::TriangleBuffer::~TriangleBuffer() {
  std::cout << "IMPLEMENT TRIANGLE BUFFER DESCTRUCTOR!" << std::endl;
}

const VkBuffer odin::TriangleBuffer::getBuffer() const { return buffer; }

const VkDeviceMemory odin::TriangleBuffer::getBufferMemory() const {
  return triangleBufferMemory;
}

const VkDescriptorBufferInfo odin::TriangleBuffer::getDescriptor() const {
  return descriptor;
}

const size_t odin::TriangleBuffer::getTriangleCount() const {
  return numTriangles;
}