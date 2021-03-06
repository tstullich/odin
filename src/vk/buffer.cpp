#include "vk/buffer.hpp"

odin::Buffer::Buffer() {}

void odin::Buffer::copyBuffer(const DeviceManager& deviceManager,
                              const CommandPool& commandPool,
                              VkBuffer srcBuffer, VkBuffer dstBuffer,
                              VkDeviceSize size) {
  // TODO Create a pre-allocated buffer pool for short-lived command buffers
  // Make sure to then use the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag
  auto commandBuffer =
      commandPool.beginSingleTimeCommands(deviceManager.getLogicalDevice());

  VkBufferCopy copyRegion = {};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  commandPool.endSingleTimeCommands(deviceManager, commandBuffer);
}

void odin::Buffer::createBuffer(const DeviceManager& deviceManager,
                                VkDeviceSize size, VkBufferUsageFlags usage,
                                VkMemoryPropertyFlags properties,
                                VkBuffer& buffer,
                                VkDeviceMemory& bufferMemory) {
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(deviceManager.getLogicalDevice(), &bufferInfo, nullptr,
                     &buffer) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(deviceManager.getLogicalDevice(), buffer,
                                &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(deviceManager.getPhysicalDevice(),
                     memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(deviceManager.getLogicalDevice(), &allocInfo, nullptr,
                       &bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate buffer memory!");
  }

  vkBindBufferMemory(deviceManager.getLogicalDevice(), buffer, bufferMemory, 0);
}

void odin::Buffer::createBuffer(VkPhysicalDevice physicalDevice,
                                VkDevice logicalDevice, VkDeviceSize size,
                                VkBufferUsageFlags usageFlags,
                                VkMemoryPropertyFlags properties,
                                VkBuffer& buffer,
                                VkDeviceMemory& bufferMemory) {
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usageFlags;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(
      physicalDevice, memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate buffer memory!");
  }

  vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
}

uint32_t odin::Buffer::Buffer::findMemoryType(
    const VkPhysicalDevice& physicalDevice, uint32_t typeFilter,
    VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags &
                                  properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("Failed to find suitable memory type!");
}