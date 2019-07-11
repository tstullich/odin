#include "renderer/ubo.hpp"
#include "vk/descriptor_pool.hpp"

odin::DescriptorPool::DescriptorPool(
    const DeviceManager& deviceManager, const Swapchain& swapChain,
    const DescriptorSetLayout& descriptorSetLayout,
    const std::vector<UniformBuffer>& uniformBuffers,
    const TextureImage& textureImage, const TextureSampler& textureSampler) {
  createDescriptorPool(deviceManager, swapChain);
  createDescriptorSets(deviceManager, swapChain, descriptorSetLayout,
                       uniformBuffers, textureImage, textureSampler);
}

odin::DescriptorPool::~DescriptorPool() {
  std::cout << "IMPLEMENT DESCRIPTOR POOL DESTRUCTOR!" << std::endl;
}

const VkDescriptorPool odin::DescriptorPool::getDescriptorPool() const {
  return descriptorPool;
}

const std::vector<VkDescriptorSet> odin::DescriptorPool::getDescriptorSets()
    const {
  return descriptorSets;
}

void odin::DescriptorPool::createDescriptorPool(
    const DeviceManager& deviceManager, const Swapchain& swapChain) {
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

void odin::DescriptorPool::createDescriptorSets(
    const DeviceManager& deviceManager, const Swapchain& swapChain,
    const DescriptorSetLayout& descriptorSetLayout,
    const std::vector<UniformBuffer>& uniformBuffers,
    const TextureImage& textureImage, const TextureSampler& textureSampler) {
  std::vector<VkDescriptorSetLayout> layouts(
      swapChain.getImageSize(), *descriptorSetLayout.getDescriptorSetLayout());
  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount =
      static_cast<uint32_t>(swapChain.getImageSize());
  allocInfo.pSetLayouts = layouts.data();

  descriptorSets.resize(swapChain.getImageSize());
  if (vkAllocateDescriptorSets(deviceManager.getLogicalDevice(), &allocInfo,
                               descriptorSets.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate descriptor sets!");
  }

  for (size_t i = 0; i < swapChain.getImageSize(); i++) {
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = uniformBuffers[i].getBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textureImage.getTextureImageView();
    imageInfo.sampler = textureSampler.getSampler();

    std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(deviceManager.getLogicalDevice(),
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }
}