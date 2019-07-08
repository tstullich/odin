#ifndef ODIN_COMMAND_POOL_HPP
#define ODIN_COMMAND_POOL_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "vk/device_manager.hpp"
#include "vk/pipeline.hpp"
#include "vk/render_pass.hpp"
#include "vk/swapchain.hpp"

namespace odin {
class CommandPool {
 public:
  CommandPool(const VkDevice& logicalDevice,
              const odin::QueueFamilyIndices& queueFamilyIndices);
  ~CommandPool();

  const VkCommandBuffer* getCommandBuffer(uint32_t bufferIndex) const;

  void createCommandBuffers(const VkDevice& logicalDevice,
                            const RenderPass& renderPass,
                            const Pipeline& graphicsPipeline,
                            const Swapchain& swapChain);

 private:
  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;
};
}  // namespace odin
#endif  // ODIN_COMMAND_POOL_HPP