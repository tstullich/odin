#ifndef ODIN_PIPELINE_HPP
#define ODIN_PIPELINE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "renderer/vertex.hpp"
#include "utils/file_reader.hpp"
#include "vk/descriptor_set_layout.hpp"
#include "vk/render_pass.hpp"
#include "vk/shader_module.hpp"
#include "vk/swapchain.hpp"

namespace odin {
class Pipeline {
 public:
  Pipeline(const VkDevice& logicalDevice, const Swapchain& swapChain,
           const RenderPass& renderPass,
           const DescriptorSetLayout& descriptorSetLayout);

  ~Pipeline();

  const VkPipeline getGraphicsPipeline() const;

  const VkPipelineLayout getPipelineLayout() const;

 private:
  void createPipeline(const VkDevice& logicalDevice, const Swapchain& swapChain,
                      const RenderPass& renderPass,
                      const DescriptorSetLayout& descriptorSetLayout);

  VkPipeline graphicsPipeline;
  VkPipelineLayout pipelineLayout;
};
}  // namespace odin
#endif  // ODIN_PIPELINE_HPP