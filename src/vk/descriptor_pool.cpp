#include "vk/descriptor_pool.hpp"
#include "renderer/ubo.hpp"

odin::DescriptorPool::DescriptorPool(
    const DeviceManager& deviceManager, const Swapchain& swapChain,
    const DescriptorSetLayout& computeDescriptorSetLayout,
    const DescriptorSetLayout& graphicsDescriptorSetLayout,
    const TextureImage& textureImage, const TextureSampler& textureSampler,
    const std::vector<VkDescriptorBufferInfo>& bufferInfos) {
  createDescriptorPool(deviceManager, swapChain);
  createComputeDescriptorSets(deviceManager, computeDescriptorSetLayout,
                              swapChain, textureImage, bufferInfos);
  createGraphicsDescriptorSets(deviceManager, graphicsDescriptorSetLayout,
                               textureImage, textureSampler);
}

odin::DescriptorPool::~DescriptorPool() {
  std::cout << "IMPLEMENT DESCRIPTOR POOL DESTRUCTOR!" << std::endl;
}

const VkDescriptorPool odin::DescriptorPool::getDescriptorPool() const {
  return descriptorPool;
}

const VkDescriptorSet odin::DescriptorPool::getDescriptorSet() const {
  return descriptorSet;
}

void odin::DescriptorPool::createComputeDescriptorSets(
    const DeviceManager& deviceManager,
    const DescriptorSetLayout& descriptorSetLayout, const Swapchain& swapChain,
    const TextureImage& textureImage,
    const std::vector<VkDescriptorBufferInfo> bufferInfos) {
  // Allocate the descriptors for the compute pipeline
  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.pSetLayouts = descriptorSetLayout.getDescriptorSetLayout();
  allocInfo.descriptorSetCount = 1;

  if (vkAllocateDescriptorSets(deviceManager.getLogicalDevice(), &allocInfo,
                               &descriptorSet) != VK_SUCCESS) {
    throw std::runtime_error(
        "Unable to allocate descriptor set for compute pipeline!");
  }

  // Buffer descriptors need to match our bind points
  if (bufferInfos.size() != BUFFER_DESCRIPTORS) {
    throw std::runtime_error("Buffer Descriptors size does not match!");
  }

  // Output image for the compute shader
  VkWriteDescriptorSet outputDescriptor = {};
  outputDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  outputDescriptor.dstSet = descriptorSet;
  outputDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  outputDescriptor.dstBinding = 0;
  outputDescriptor.pImageInfo = &textureImage.getDescriptor();
  outputDescriptor.descriptorCount = 1;

  // Uniform Buffer Object for various data
  VkWriteDescriptorSet uboDescriptor = {};
  uboDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  uboDescriptor.dstSet = descriptorSet;
  uboDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboDescriptor.dstBinding = 1;
  uboDescriptor.pBufferInfo = &bufferInfos[0];
  uboDescriptor.descriptorCount = 1;

  // Triangle data for performing pathtracing in the compute shader
  VkWriteDescriptorSet triangleDescriptor = {};
  triangleDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  triangleDescriptor.dstSet = descriptorSet;
  triangleDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  triangleDescriptor.dstBinding = 2;
  triangleDescriptor.pBufferInfo = &bufferInfos[1];
  triangleDescriptor.descriptorCount = 1;

  std::array<VkWriteDescriptorSet, 3> computeWriteDescriptorSets = {
      outputDescriptor, uboDescriptor, triangleDescriptor};

  vkUpdateDescriptorSets(deviceManager.getLogicalDevice(),
                         computeWriteDescriptorSets.size(),
                         computeWriteDescriptorSets.data(), 0, nullptr);
}

void odin::DescriptorPool::createDescriptorPool(
    const DeviceManager& deviceManager, const Swapchain& swapChain) {
  // Need to match the amount of descriptors we have for the descriptor layout
  std::array<VkDescriptorPoolSize, 4> poolSizes = {};
  // Pool size for UBOs
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = 2;
  // Pool size for graphics pipeline image sampler
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = 4;
  // Storage image for raytraced result
  poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  poolSizes[2].descriptorCount = 1;
  // Storage buffer for scene primitives
  poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSizes[3].descriptorCount = 2;

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = 3;

  if (vkCreateDescriptorPool(deviceManager.getLogicalDevice(), &poolInfo,
                             nullptr, &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor pool!");
  }
}

void odin::DescriptorPool::createGraphicsDescriptorSets(
    const DeviceManager& deviceManager,
    const DescriptorSetLayout& descriptorSetLayout,
    const TextureImage& textureImage, const TextureSampler& textureSampler) {
  VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
  descriptorSetAllocateInfo.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  descriptorSetAllocateInfo.descriptorPool = descriptorPool;
  descriptorSetAllocateInfo.pSetLayouts =
      descriptorSetLayout.getDescriptorSetLayout();
  descriptorSetAllocateInfo.descriptorSetCount = 1;

  if (vkAllocateDescriptorSets(deviceManager.getLogicalDevice(),
                               &descriptorSetAllocateInfo,
                               &descriptorSet) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate graphics descriptor sets!");
  }

  VkWriteDescriptorSet writeDescriptor = {};
  writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeDescriptor.dstSet = descriptorSet;
  writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  writeDescriptor.dstBinding = 0;
  writeDescriptor.pImageInfo = &textureImage.getDescriptor();
  writeDescriptor.descriptorCount = 1;

  std::array<VkWriteDescriptorSet, 1> writeDescriptorSets = {writeDescriptor};
  vkUpdateDescriptorSets(deviceManager.getLogicalDevice(),
                         writeDescriptorSets.size(), writeDescriptorSets.data(),
                         0, nullptr);
}