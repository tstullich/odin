#include "vk/render_pass.hpp"

odin::RenderPass::RenderPass(const VkDevice& logicalDevice,
                             const VkFormat& imageFormat,
                             const VkFormat& depthFormat) {
  createRenderPass(logicalDevice, imageFormat, depthFormat);
}

odin::RenderPass::~RenderPass() {
  // TODO Implement
  std::cout << "IMPLEMENT RENDER PASS DESTRUCTOR!" << std::endl;
}

void odin::RenderPass::createRenderPass(const VkDevice& logicalDevice,
                                        const VkFormat& imageFormat,
                                        const VkFormat& depthFormat) {
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = imageFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  // Clear the framebuffer before and after rendering
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  // Store color data in memory for future read operations
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  // Configure if stencil data should be stored
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentDescription depthAttachment = {};
  depthAttachment.format = depthFormat;
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // Configure subpasses. For now we just need the two for rendering and depth
  // testing
  VkAttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef = {};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  // This can be directly referenced in our fragment shader
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(logicalDevice, &renderPassInfo,
                         nullptr, &renderPass) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create render pass!");
  }
}

const VkRenderPass odin::RenderPass::getRenderPass() const {
  return renderPass;
}