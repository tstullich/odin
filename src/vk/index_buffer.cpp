#include "vk/index_buffer.hpp"

odin::IndexBuffer::IndexBuffer(const VkDevice& logicalDevice,
                               const VkPhysicalDevice& physicalDevice,
                               const std::vector<uint32_t>& indices) {
  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(physicalDevice, logicalDevice, bufferSize,
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);

  void* data;
  vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(logicalDevice, stagingBufferMemory);

  createBuffer(
      physicalDevice, logicalDevice, bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, indexBufferMemory);

  copyBuffer(stagingBuffer, buffer, bufferSize);

  // Destroy the staging buffer and backing memory once we successfully copied
  // our vertex data into GPU memory
  vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

odin::IndexBuffer::~IndexBuffer() {}