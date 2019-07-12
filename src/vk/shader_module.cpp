#include "vk/shader_module.hpp"

odin::ShaderModule::ShaderModule(const VkDevice& logicalDevice,
                   const std::vector<char>& shaderCode) {
  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = shaderCode.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

  if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr,
                           &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create shader module!");
  }

  ownedDevice = logicalDevice;
}

odin::ShaderModule::~ShaderModule() {
  vkDestroyShaderModule(ownedDevice, shaderModule, nullptr);
}

const VkShaderModule odin::ShaderModule::getShaderModule() const {
  return shaderModule;
}