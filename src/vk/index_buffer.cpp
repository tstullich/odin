#include "vk/index_buffer.hpp"
#include "vk/command_pool.hpp"

odin::IndexBuffer::IndexBuffer(const DeviceManager& deviceManager,
                               const CommandPool& commandPool,
                               const std::vector<uint32_t>& indices) {
  numIndices = indices.size();
  VkDeviceSize bufferSize = sizeof(indices[0]) * numIndices;

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
  memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(deviceManager.getLogicalDevice(), stagingBufferMemory);

  createBuffer(
      deviceManager.getPhysicalDevice(), deviceManager.getLogicalDevice(),
      bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, indexBufferMemory);

  copyBuffer(deviceManager, commandPool, stagingBuffer, buffer, bufferSize);

  // Destroy the staging buffer and backing memory once we successfully copied
  // our vertex data into GPU memory
  vkDestroyBuffer(deviceManager.getLogicalDevice(), stagingBuffer, nullptr);
  vkFreeMemory(deviceManager.getLogicalDevice(), stagingBufferMemory, nullptr);
}

odin::IndexBuffer::~IndexBuffer() {
  std::cout << "MUST IMPLEMENT INDEX BUFFER DESTRUCTOR!" << std::endl;
}

const VkBuffer odin::IndexBuffer::getBuffer() const { return buffer; }

const VkDeviceMemory odin::IndexBuffer::getBufferMemory() const {
  return indexBufferMemory;
}

const uint32_t odin::IndexBuffer::getNumIndices() const {
  return numIndices;
}