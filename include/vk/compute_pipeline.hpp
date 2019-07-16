#ifndef ODIN_COMPUTE_PIPELINE_HPP
#define ODIN_COMPUTE_PIPELINE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>

#include "vk/device_manager.hpp"

namespace odin {
class ComputePipeline {
 public:
  ComputePipeline(const DeviceManager& deviceManager);

  ~ComputePipeline();

  const VkPipeline getComputePipeline() const;

  const VkPipelineLayout getPipelineLayout() const;

  private:
  void createPipeline(const DeviceManager& deviceManager);

  VkPipeline computePipeline;
  VkPipelineLayout pipelineLayout;
};
}  // namespace odin
#endif // ODIN_COMPUTE_PIPELINE_HPP