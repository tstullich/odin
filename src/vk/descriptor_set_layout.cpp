#include "vk/descriptor_set_layout.hpp"

odin::DescriptorSetLayout::DescriptorSetLayout(
    const DeviceManager& deviceManager, bool computePipeline) {
  if (computePipeline) {
    createComputeDescriptorSetLayout(deviceManager);
  } else {
    createGraphicsDescriptorSetLayout(deviceManager);
  }
}

odin::DescriptorSetLayout::~DescriptorSetLayout() {
  std::cout << "IMPLEMENT DESCRIPTOR SET LAYOUT DESTRUCTOR!" << std::endl;
}

void odin::DescriptorSetLayout::createComputeDescriptorSetLayout(
    const DeviceManager& deviceManager) {
  // Binding for the raytraced output
  VkDescriptorSetLayoutBinding outputBinding = {};
  outputBinding.binding = 0;
  outputBinding.descriptorCount = 1;
  outputBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  outputBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  // Binding for the Uniform Buffer Object
  VkDescriptorSetLayoutBinding uboBinding = {};
  outputBinding.binding = 1;
  outputBinding.descriptorCount = 1;
  outputBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  outputBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  // TODO Implement Triangle struct for storage
  VkDescriptorSetLayoutBinding triangleBinding = {};
  outputBinding.binding = 2;
  outputBinding.descriptorCount = 1;
  outputBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  outputBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  std::array<VkDescriptorSetLayoutBinding, 3> bindings = {
      outputBinding, uboBinding, triangleBinding};

  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(deviceManager.getLogicalDevice(), &layoutInfo,
                                  nullptr,
                                  &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor set layout!");
  }
}

void odin::DescriptorSetLayout::createGraphicsDescriptorSetLayout(
    const DeviceManager& deviceManager) {
  // This layout specifies the sampler that will be used in
  // the fragment shader
  VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
  samplerLayoutBinding.binding = 0;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 1> bindings = {samplerLayoutBinding};

  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(deviceManager.getLogicalDevice(), &layoutInfo,
                                  nullptr,
                                  &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor set layout!");
  }
}

const VkDescriptorSetLayout* odin::DescriptorSetLayout::getDescriptorSetLayout()
    const {
  return &descriptorSetLayout;
}