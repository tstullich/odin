#ifndef ODIN_GRAPHICS_PIPELINE_HPP
#define ODIN_GRAPHICS_PIPELINE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

#include "renderer/vertex.hpp"
#include "utils/file_reader.hpp"
#include "vk/descriptor_set_layout.hpp"
#include "vk/render_pass.hpp"
#include "vk/shader_module.hpp"
#include "vk/swapchain.hpp"

namespace odin {
class GraphicsPipeline {
 public:
  GraphicsPipeline(const VkDevice& logicalDevice, const Swapchain& swapChain,
                   const RenderPass& renderPass,
                   const DescriptorSetLayout& descriptorSetLayout,
                   const std::string& vertexShaderPath,
                   const std::string& fragmentShaderPath);

  const VkPipeline getGraphicsPipeline() const;

  const VkPipelineLayout getPipelineLayout() const;

 private:
  void createPipeline(const VkDevice& logicalDevice, const Swapchain& swapChain,
                      const RenderPass& renderPass,
                      const DescriptorSetLayout& descriptorSetLayout,
                      const std::string& vertexShaderPath,
                      const std::string& fragmentShaderPath);

  VkPipeline graphicsPipeline;
  VkPipelineLayout pipelineLayout;
};
}  // namespace odin
#endif  // ODIN_GRAPHICS_PIPELINE_HPP