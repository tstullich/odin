#include "vk/uniform_buffer.hpp"

odin::UniformBuffer::UniformBuffer(const DeviceManager& deviceManager,
                                   const VkDeviceSize bufferSize) {
  createBuffer(deviceManager.getPhysicalDevice(),
               deviceManager.getLogicalDevice(), bufferSize,
               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               buffer, uniformBufferMemory);

  // Setup a descriptor for the buffer
  descriptor.offset = 0;
  descriptor.buffer = buffer;
  descriptor.range = VK_WHOLE_SIZE;
}

const VkBuffer odin::UniformBuffer::getBuffer() const { return buffer; }

const VkDescriptorBufferInfo odin::UniformBuffer::getDescriptor() const {
  return descriptor;
}

const VkDeviceMemory odin::UniformBuffer::getDeviceMemory() const {
  return uniformBufferMemory;
}