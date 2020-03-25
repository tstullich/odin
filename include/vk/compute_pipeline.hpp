#ifndef ODIN_COMPUTE_PIPELINE_HPP
#define ODIN_COMPUTE_PIPELINE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include "utils/file_reader.hpp"
#include "vk/descriptor_set_layout.hpp"
#include "vk/device_manager.hpp"
#include "vk/shader_module.hpp"

namespace odin {
class ComputePipeline {
 public:
  ComputePipeline(const DeviceManager& deviceManager,
                  const DescriptorSetLayout& descriptorSetLayout,
                  const std::string& computeShaderPath);

  const VkPipeline getComputePipeline() const;

  const VkPipelineLayout getPipelineLayout() const;

 private:
  void createPipeline(const DeviceManager& deviceManager,
                      const DescriptorSetLayout& descriptorSetLayout,
                      const std::string& computeShaderPath);

  VkPipeline computePipeline;
  VkPipelineLayout pipelineLayout;
};
}  // namespace odin
#endif  // ODIN_COMPUTE_PIPELINE_HPP