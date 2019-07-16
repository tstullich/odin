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
class IndexBuffer;
class GraphicsPipeline;
class VertexBuffer;

class CommandPool {
 public:
  CommandPool(const VkDevice& logicalDevice,
              const odin::QueueFamilyIndices& queueFamilyIndices);

  ~CommandPool();

  const VkCommandBuffer beginSingleTimeCommands(
      const VkDevice& logicalDevice) const;

  void createCommandBuffers(const VkDevice& logicalDevice,
                            const RenderPass& renderPass,
                            const GraphicsPipeline& graphicsPipeline,
                            const Swapchain& swapChain,
                            const IndexBuffer& indexBuffer,
                            const VertexBuffer& vertexBuffer,
                            const std::vector<VkDescriptorSet>& descriptorSets);

  void endSingleTimeCommands(const DeviceManager& deviceManager,
                             VkCommandBuffer commandBuffer) const;

  const VkCommandBuffer* getCommandBuffer(uint32_t bufferIndex) const;

  const std::vector<VkCommandBuffer> getCommandBuffers() const;

  const size_t getCommandBufferSize() const;

  const VkCommandPool getCommandPool() const;

 private:
  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;
};
}  // namespace odin
#endif  // ODIN_COMMAND_POOL_HPP