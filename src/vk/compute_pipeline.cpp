#include "vk/compute_pipeline.hpp"

odin::ComputePipeline::ComputePipeline(
    const DeviceManager& deviceManager,
    const DescriptorSetLayout& descriptorSetLayout,
    const std::string& computeShaderPath) {
  createPipeline(deviceManager, descriptorSetLayout, computeShaderPath);
}

odin::ComputePipeline::~ComputePipeline() {
  std::cout << "IMPLEMENT COMPUTE PIPELINE DESTRUCTOR!" << std::endl;
}

const VkPipeline odin::ComputePipeline::getComputePipeline() const {
  return computePipeline;
}

const VkPipelineLayout odin::ComputePipeline::getPipelineLayout() const {
  return pipelineLayout;
}

void odin::ComputePipeline::createPipeline(
    const DeviceManager& deviceManager,
    const DescriptorSetLayout& descriptorSetLayout,
    const std::string& computeShaderPath) {
  // Load compute shader
  auto computeShaderCode =
      FileReader::readFile(computeShaderPath);

  // Create compute shader module
  ShaderModule computeShaderModule(deviceManager.getLogicalDevice(),
                                   computeShaderCode);

  // Setup compute shader stage
  VkPipelineShaderStageCreateInfo computeShaderStageStageInfo = {};
  computeShaderStageStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  computeShaderStageStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  computeShaderStageStageInfo.module = computeShaderModule.getShaderModule();
  computeShaderStageStageInfo.pName = "main";

  // Setup compute pipeline layout
  VkPipelineLayoutCreateInfo pipelineLayoutInfo;
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.getDescriptorSetLayout();
  pipelineLayoutInfo.flags = 0;

  if (vkCreatePipelineLayout(deviceManager.getLogicalDevice(),
                             &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create compute pipeline layout!");
  }

  // Setup compute pipeline
  VkComputePipelineCreateInfo pipelineCreateInfo = {};
  pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipelineCreateInfo.layout = pipelineLayout;
  pipelineCreateInfo.flags = 0;
  pipelineCreateInfo.stage = computeShaderStageStageInfo;

  if (vkCreateComputePipelines(deviceManager.getLogicalDevice(), VK_NULL_HANDLE,
                               1, &pipelineCreateInfo, nullptr,
                               &computePipeline) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create compute pipeline!");
  }
}