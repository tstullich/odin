#include "vk/command_pool.hpp"
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
  std::cout << "MUST IMPLEMENT COMMAND POOL DESTRUCTOR!" << std::endl;
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

void odin::CommandPool::createCommandBuffers(
    const VkDevice& logicalDevice, const RenderPass& renderPass,
    const Pipeline& graphicsPipeline, const Swapchain& swapChain,
    const IndexBuffer& indexBuffer, const VertexBuffer& vertexBuffer,
    const std::vector<VkDescriptorSet>& descriptorSets) {
  commandBuffers.resize(swapChain.getFrameBufferSizes());

  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(logicalDevice, &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffers!");
  }

  // Record buffer commands
  for (size_t i = 0; i < commandBuffers.size(); i++) {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass.getRenderPass();
    renderPassInfo.framebuffer = swapChain.getFrameBuffer(i);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain.getExtent();

    // Because we have multiple attachments with VK_ATTACHMENT_LOAD_OP_CLEAR
    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0] = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    // Specify render pass commands
    vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    // Bind pipeline to command buffers
    vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphicsPipeline.getGraphicsPipeline());

    // Bind the vertex buffer to the pipeline
    VkBuffer vertexBuffers[] = {vertexBuffer.getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

    // Bind the vertex indices buffer to the pipeline
    vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer.getBuffer(), 0,
                         VK_INDEX_TYPE_UINT32);

    // Bind descriptor sets to swap chain images
    vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                            graphicsPipeline.getPipelineLayout(), 0, 1,
                            &descriptorSets[i], 0, nullptr);

    // Draw the vertices using the indices
    vkCmdDrawIndexed(commandBuffers[i],
                     static_cast<uint32_t>(indexBuffer.getNumIndices()), 1, 0,
                     0, 0);

    // End render pass
    vkCmdEndRenderPass(commandBuffers[i]);

    if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to record command buffer");
    }
  }
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