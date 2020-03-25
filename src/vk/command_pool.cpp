#include "vk/command_pool.hpp"

#include "vk/compute_pipeline.hpp"
#include "vk/descriptor_pool.hpp"
#include "vk/graphics_pipeline.hpp"
#include "vk/texture_image.hpp"

odin::CommandPool::CommandPool(
    const VkDevice& logicalDevice,
    const odin::QueueFamilyIndices& queueFamilyIndices) {
  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();
  poolInfo.flags = 0;

  if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr,
                          &computeCommandPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create compute command pool!");
  }

  // Create separate queue for graphics since indices might be different
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
  if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr,
                          &graphicsCommandPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create graphics command pool!");
  }
}

const VkCommandBuffer odin::CommandPool::beginSingleTimeCommands(
    const VkDevice& logicalDevice) const {
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = graphicsCommandPool;
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
    const ComputePipeline& computePipeline,
    const DescriptorPool& descriptorPool, uint32_t texWidth,
    uint32_t texHeight) {
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = computeCommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = 1;

  if (vkAllocateCommandBuffers(logicalDevice, &allocInfo,
                               &computeCommandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffers!");
  }

  VkCommandBufferBeginInfo commandBufferInfo = {};
  commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(computeCommandBuffer, &commandBufferInfo) !=
      VK_SUCCESS) {
    throw std::runtime_error(
        "Unable to begin recording compute command buffer commands!");
  }

  // Record commands for the compute pipeline
  vkCmdBindPipeline(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    computePipeline.getComputePipeline());
  vkCmdBindDescriptorSets(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          computePipeline.getPipelineLayout(), 0, 1,
                          descriptorPool.getComputeDescriptorSet(), 0, 0);
  // Break up raytracing task into work groups
  vkCmdDispatch(computeCommandBuffer, texWidth / WORK_GROUP_SIZE,
                texHeight / WORK_GROUP_SIZE, 1);

  vkEndCommandBuffer(computeCommandBuffer);
}

void odin::CommandPool::createGraphicsCommandBuffers(
    const VkDevice& logicalDevice, const RenderPass& renderPass,
    const GraphicsPipeline& graphicsPipeline,
    const DescriptorPool& descriptorPool, const Swapchain& swapChain,
    const TextureImage& texture) {
  // Allocate command buffers first
  graphicsCommandBuffers.resize(swapChain.getFrameBufferSizes());
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = graphicsCommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount =
      static_cast<uint32_t>(graphicsCommandBuffers.size());

  if (vkAllocateCommandBuffers(logicalDevice, &allocInfo,
                               graphicsCommandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate graphics command buffers!");
  }

  VkCommandBufferBeginInfo commandBufferInfo = {};
  commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  VkClearValue clearValues[2];
  clearValues[0].color = {0.025f, 0.025f, 0.025f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};

  VkRenderPassBeginInfo renderPassBeginInfo = {};
  renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassBeginInfo.renderPass = renderPass.getRenderPass();
  renderPassBeginInfo.renderArea.offset.x = 0;
  renderPassBeginInfo.renderArea.offset.y = 0;
  renderPassBeginInfo.renderArea.extent.height = texture.getHeight();
  renderPassBeginInfo.renderArea.extent.width = texture.getWidth();
  renderPassBeginInfo.clearValueCount = 2;
  renderPassBeginInfo.pClearValues = clearValues;

  for (uint32_t i = 0; i < graphicsCommandBuffers.size(); i++) {
    renderPassBeginInfo.framebuffer = swapChain.getFrameBuffer(i);

    if (vkBeginCommandBuffer(graphicsCommandBuffers[i], &commandBufferInfo) !=
        VK_SUCCESS) {
      throw std::runtime_error(
          "Unable to start recording graphics framebuffer!");
    }

    // Put a barrier here to make sure compute shader writes finish before
    // sampling
    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageMemoryBarrier.image = texture.getTextureImage();
    imageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0,
                                           1};
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(graphicsCommandBuffers[i],
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                         0, nullptr, 1, &imageMemoryBarrier);

    vkCmdBeginRenderPass(graphicsCommandBuffers[i], &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {};
    viewport.height = static_cast<float>(texture.getHeight());
    viewport.width = static_cast<float>(texture.getWidth());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(graphicsCommandBuffers[i], 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.extent.height = texture.getHeight();
    scissor.extent.width = texture.getWidth();
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(graphicsCommandBuffers[i], 0, 1, &scissor);

    vkCmdBindDescriptorSets(
        graphicsCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipeline.getPipelineLayout(), 0, 1,
        descriptorPool.getGraphicsDescriptorSet(), 0, nullptr);

    vkCmdBindPipeline(graphicsCommandBuffers[i],
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphicsPipeline.getGraphicsPipeline());

    vkCmdDraw(graphicsCommandBuffers[i], 3, 1, 0, 0);

    vkCmdEndRenderPass(graphicsCommandBuffers[i]);

    if (vkEndCommandBuffer(graphicsCommandBuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error(
          "Unable to end recording of graphics command buffer!");
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

  vkFreeCommandBuffers(deviceManager.getLogicalDevice(), graphicsCommandPool, 1,
                       &commandBuffer);
}

const VkCommandBuffer* odin::CommandPool::getComputeCommandBuffer() const {
  return &computeCommandBuffer;
}

const VkCommandPool odin::CommandPool::getComputeCommandPool() const {
  return computeCommandPool;
}

const VkCommandBuffer* odin::CommandPool::getGraphicsCommandBuffer(
    uint32_t bufferIndex) const {
  return &graphicsCommandBuffers[bufferIndex];
}

const std::vector<VkCommandBuffer>
odin::CommandPool::getGraphicsCommandBuffers() const {
  return graphicsCommandBuffers;
}

const size_t odin::CommandPool::getGraphicsCommandBufferSize() const {
  return graphicsCommandBuffers.size();
}

const VkCommandPool odin::CommandPool::getGraphicsCommandPool() const {
  return graphicsCommandPool;
}