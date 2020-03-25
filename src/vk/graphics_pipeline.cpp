#include "vk/graphics_pipeline.hpp"

odin::GraphicsPipeline::GraphicsPipeline(
    const VkDevice& logicalDevice, const Swapchain& swapChain,
    const RenderPass& renderPass,
    const DescriptorSetLayout& descriptorSetLayout,
    const std::string& vertexShaderPath,
    const std::string& fragmentShaderPath) {
  createPipeline(logicalDevice, swapChain, renderPass, descriptorSetLayout,
                 vertexShaderPath, fragmentShaderPath);
}

void odin::GraphicsPipeline::createPipeline(
    const VkDevice& logicalDevice, const Swapchain& swapChain,
    const RenderPass& renderPass,
    const DescriptorSetLayout& descriptorSetLayout,
    const std::string& vertexShaderPath,
    const std::string& fragmentShaderPath) {
  // Load shaders from file
  auto vertShaderCode = FileReader::readFile(vertexShaderPath);
  auto fragShaderCode = FileReader::readFile(fragmentShaderPath);

  // Create shader module wrappers
  ShaderModule vertShaderModule(logicalDevice, vertShaderCode);
  ShaderModule fragShaderModule(logicalDevice, fragShaderCode);

  // Setup Vertex Stage
  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule.getShaderModule();
  vertShaderStageInfo.pName = "main";

  // Setup Fragment Stage
  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule.getShaderModule();
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};

  // Get vertex binding and attribute descriptions for shaders
  // auto bindingDescription = odin::Vertex::getBindingDescription();
  // auto attributeDescriptions = odin::Vertex::getAttributeDescriptions();

  // Configure how vertices are treated in the pipeline
  // Here we simply want an empty input since the actual vertices are
  // only needed in the compute shader
  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions = nullptr;
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.pVertexBindingDescriptions = nullptr;

  // Configure vertex layout. Treat three consecutive vertices as a triangle
  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.flags = 0;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  // Combine viewport and scissor into a viewport state
  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.flags = 0;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  // Configure the rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  // Set this to VK_TRUE in case you want to use shadow maps
  // Geometry will then never pass through the rasterizer stage
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  // Enables multisampling for anti-aliasing but it is disabled for now
  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.flags = 0;

  // Enables depth testing
  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_FALSE;
  depthStencil.depthWriteEnable = VK_FALSE;
  // The lower the depth value the close it is to the viewer
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  depthStencil.front = depthStencil.back;
  depthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;

  // Configure color blending based on one framebuffer. For multiple
  // framebuffers other settings need to be enabled. Right now it is
  // disabled but can be useful for alpha blending later on
  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask = VK_BLEND_FACTOR_SRC1_COLOR;
  colorBlendAttachment.blendEnable = VK_FALSE;

  // Set constants for final color blending. Disabled for now
  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;

  // Setup dynamic states for the viewport and scissor
  std::array<VkDynamicState, 2> dynamicStateEnables = {
      VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicStates = {};
  dynamicStates.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStates.pDynamicStates = dynamicStateEnables.data();
  dynamicStates.dynamicStateCount =
      static_cast<uint32_t>(dynamicStateEnables.size());
  dynamicStates.flags = 0;

  // Create our pipeline layout. This is also the point where later on
  // uniforms can be bound for use in our shaders
  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.getDescriptorSetLayout();

  if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create graphics pipeline layout!");
  }

  // Combine all components into our final graphics pipeline
  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pDynamicState = &dynamicStates;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.renderPass = renderPass.getRenderPass();
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.pNext = VK_NULL_HANDLE;

  if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo,
                                nullptr, &graphicsPipeline) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create graphics pipeline!");
  }
}

const VkPipeline odin::GraphicsPipeline::getGraphicsPipeline() const {
  return graphicsPipeline;
}

const VkPipelineLayout odin::GraphicsPipeline::getPipelineLayout() const {
  return pipelineLayout;
}