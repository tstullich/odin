#include "vk/bvh_buffer.hpp"
#include "vk/command_pool.hpp"

odin::BvhBuffer::BvhBuffer(const DeviceManager &deviceManager,
                           const CommandPool &commandPool,
                           const std::vector<BVH_Node> &nodes) {
  numNodes = nodes.size();
  VkDeviceSize bufferSize = sizeof(nodes[0]) * numNodes;
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(deviceManager.getPhysicalDevice(),
               deviceManager.getLogicalDevice(), bufferSize,
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(deviceManager.getLogicalDevice(), stagingBufferMemory, 0,
              bufferSize, 0, &data);
  memcpy(data, nodes.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(deviceManager.getLogicalDevice(), stagingBufferMemory);

  createBuffer(deviceManager.getPhysicalDevice(),
               deviceManager.getLogicalDevice(), bufferSize,
               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                   VK_BUFFER_USAGE_TRANSFER_DST_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bvhBufferMemory);

  copyBuffer(deviceManager, commandPool, stagingBuffer, buffer, bufferSize);

  vkDestroyBuffer(deviceManager.getLogicalDevice(), stagingBuffer, nullptr);
  vkFreeMemory(deviceManager.getLogicalDevice(), stagingBufferMemory, nullptr);

  // Setup descriptor
  descriptor.offset = 0;
  descriptor.buffer = buffer;
  descriptor.range = VK_WHOLE_SIZE;
}

odin::BvhBuffer::~BvhBuffer() {
  std::cout << "IMPLEMENT BVH BUFFER DESCTRUCTOR!" << std::endl;
}

const VkBuffer odin::BvhBuffer::getBuffer() const { return buffer; }

const VkDeviceMemory odin::BvhBuffer::getBufferMemory() const {
  return bvhBufferMemory;
}

const VkDescriptorBufferInfo odin::BvhBuffer::getDescriptor() const {
  return descriptor;
}

const size_t odin::BvhBuffer::getNodeCount() const {
  return numNodes;
}