#include "vk/texture_sampler.hpp"

odin::TextureSampler::TextureSampler(const DeviceManager& deviceManager) {
  VkSamplerCreateInfo samplerInfo = {};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  samplerInfo.addressModeV = samplerInfo.addressModeU;
  samplerInfo.addressModeW = samplerInfo.addressModeU;
  samplerInfo.maxAnisotropy = 1.0f;
  samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;

  if (vkCreateSampler(deviceManager.getLogicalDevice(), &samplerInfo, nullptr,
                      &textureSampler) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create texture sampler!");
  }
}

odin::TextureSampler::~TextureSampler() {
  std::cout << "IMPLEMENT TEXTURE SAMPLER DESCTRUCTOR!" << std::endl;
}

const VkSampler odin::TextureSampler::getSampler() const {
  return textureSampler;
}