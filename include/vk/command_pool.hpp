#ifndef ODIN_COMMAND_POOL_HPP
#define ODIN_COMMAND_POOL_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "vk/device_manager.hpp"
#include "vk/render_pass.hpp"
#include "vk/swapchain.hpp"

namespace odin {

// Forward declarations
class ComputePipeline;
class DescriptorPool;
class GraphicsPipeline;
class TextureImage;

class CommandPool {
 public:
  CommandPool(const VkDevice& logicalDevice,
              const odin::QueueFamilyIndices& queueFamilyIndices);

  ~CommandPool();

  const VkCommandBuffer beginSingleTimeCommands(
      const VkDevice& logicalDevice) const;

  void createComputeCommandBuffers(const VkDevice& logicalDevice,
                                   const RenderPass& renderPass,
                                   const ComputePipeline& computePipeline,
                                   const DescriptorPool& descriptorPool,
                                   uint32_t texWidth, uint32_t texHeight);

  void createGraphicsCommandBuffers(const VkDevice& logicalDevice,
                                    const RenderPass& renderPass,
                                    const GraphicsPipeline& graphicsPipeline,
                                    const DescriptorPool& descriptorPool,
                                    const Swapchain& swapChain,
                                    const TextureImage& texture);

  void endSingleTimeCommands(const DeviceManager& deviceManager,
                             VkCommandBuffer commandBuffer) const;

  const VkCommandBuffer* getComputeCommandBuffer() const;

  const VkCommandPool getComputeCommandPool() const;

  const VkCommandBuffer* getGraphicsCommandBuffer(uint32_t bufferIndex) const;

  const std::vector<VkCommandBuffer> getGraphicsCommandBuffers() const;

  const size_t getGraphicsCommandBufferSize() const;

  const VkCommandPool getGraphicsCommandPool() const;

 private:
  const uint32_t WORK_GROUP_SIZE = 16;
  VkCommandPool computeCommandPool;
  VkCommandPool graphicsCommandPool;
  VkCommandBuffer computeCommandBuffer;
  std::vector<VkCommandBuffer> graphicsCommandBuffers;
};
}  // namespace odin
#endif  // ODIN_COMMAND_POOL_HPP