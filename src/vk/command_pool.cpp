#include "vk/command_pool.hpp"
#include "vk/graphics_pipeline.hpp"
#include "vk/index_buffer.hpp"
#include "vk/vertex_buffer.hpp"

odin::CommandPool::CommandPool(
    const VkDevice& logicalDevice,
    const odin::QueueFamilyIndices& queueFamilyIndices) {
  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
  poolInfo.flags = 0;

  if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create command pool");
  }
}

odin::CommandPool::~CommandPool() {
  std::cout << "IMPLEMENT COMMAND POOL DESTRUCTOR!" << std::endl;
}

const VkCommandBuffer odin::CommandPool::beginSingleTimeCommands(
    const VkDevice& logicalDevice) const {
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

void odin::CommandPool::createComputeCommandBuffers(
    const VkDevice& logicalDevice, const RenderPass& renderPass,
    const GraphicsPipeline& graphicsPipeline, const IndexBuffer& indexBuffer,
    const VertexBuffer& vertexBuffer,
    const std::vector<VkDescriptorSet>& descriptorSets) {
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(logicalDevice, &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffers!");
  }

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

}

void odin::CommandPool::endSingleTimeCommands(
    const DeviceManager& deviceManager, VkCommandBuffer commandBuffer) const {
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(deviceManager.getGraphicsQueue(), 1, &submitInfo,
                VK_NULL_HANDLE);

  vkQueueWaitIdle(deviceManager.getGraphicsQueue());

  vkFreeCommandBuffers(deviceManager.getLogicalDevice(), commandPool, 1,
                       &commandBuffer);
}

const VkCommandBuffer* odin::CommandPool::getCommandBuffer(
    uint32_t bufferIndex) const {
  return &commandBuffers[bufferIndex];
}

const std::vector<VkCommandBuffer> odin::CommandPool::getCommandBuffers()
    const {
  return commandBuffers;
}

const size_t odin::CommandPool::getCommandBufferSize() const {
  return commandBuffers.size();
}

const VkCommandPool odin::CommandPool::getCommandPool() const {
  return commandPool;
}