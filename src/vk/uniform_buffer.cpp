#include "vk/uniform_buffer.hpp"

odin::UniformBuffer::UniformBuffer(const DeviceManager& deviceManager,
                                   const size_t swapChainImageSize,
                                   const VkDeviceSize bufferSize) {
  createBuffer(deviceManager.getPhysicalDevice(),
               deviceManager.getLogicalDevice(), bufferSize,
               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               buffer, uniformBufferMemory);
}

odin::UniformBuffer::~UniformBuffer() {
  std::cout << "IMPLEMENT UNIFORM BUFFER DESTRUCTOR!" << std::endl;
}

const VkBuffer odin::UniformBuffer::getBuffer() const { return buffer; }

const VkDeviceMemory odin::UniformBuffer::getDeviceMemory() const {
  return uniformBufferMemory;
}