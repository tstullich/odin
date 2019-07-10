#include "vk/descriptor_pool.hpp"

odin::DescriptorPool::DescriptorPool(const DeviceManager& deviceManager,
                                     const Swapchain& swapChain) {
  // Need to match the amount of descriptors we have for the descriptor layout
  std::array<VkDescriptorPoolSize, 2> poolSizes = {};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount =
      static_cast<uint32_t>(swapChain.getImageSize());
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount =
      static_cast<uint32_t>(swapChain.getImageSize());

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = static_cast<uint32_t>(swapChain.getImageSize());

  if (vkCreateDescriptorPool(deviceManager.getLogicalDevice(), &poolInfo,
                             nullptr, &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor pool!");
  }
}

odin::DescriptorPool::~DescriptorPool() {
  std::cout << "IMPLEMENT DESCRIPTOR POOL DESTRUCTOR!" << std::endl;
}

const VkDescriptorPool odin::DescriptorPool::getDescriptorPool() const {
  return descriptorPool;
}