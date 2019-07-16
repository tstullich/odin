#include "vk/compute_pipeline.hpp"

odin::ComputePipeline::ComputePipeline(const DeviceManager& deviceManager) {
  createPipeline(deviceManager);
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

void odin::ComputePipeline::createPipeline(const DeviceManager& deviceManager) {
  // Setup compute shader layout
  VkPipelineLayoutCreateInfo layoutCreateInfo;

  // Setup compute pipeline
  VkComputePipelineCreateInfo pipelineCreateInfo = {};
  pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipelineCreateInfo.layout = pipelineLayout;

  if (vkCreateComputePipelines(deviceManager.getLogicalDevice(), VK_NULL_HANDLE, 1,
                               &pipelineCreateInfo, nullptr,
                               &computePipeline) != VK_SUCCESS) {
    throw std::runtime_error("Could not create compute pipeline!");
  }
}