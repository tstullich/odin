#ifndef ODIN_DESCRIPTOR_POOL_HPP
#define ODIN_DESCRIPTOR_POOL_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "vk/descriptor_set_layout.hpp"
#include "vk/device_manager.hpp"
#include "vk/swapchain.hpp"
#include "vk/texture_image.hpp"
#include "vk/texture_sampler.hpp"

namespace odin {
class DescriptorPool {
 public:
  DescriptorPool(const DeviceManager& deviceManager, const Swapchain& swapChain,
                 const DescriptorSetLayout& computeDescriptorSetLayout,
                 const DescriptorSetLayout& graphicsDescriptorSetLayout,
                 const TextureImage& textureImage,
                 const TextureSampler& textureSampler,
                 const std::vector<VkDescriptorBufferInfo>& bufferInfos);

  ~DescriptorPool();

  const VkDescriptorPool getDescriptorPool() const;

  const VkDescriptorSet getDescriptorSet() const;

 private:
  void createComputeDescriptorSets(
      const DeviceManager& deviceManager,
      const DescriptorSetLayout& descriptorSetLayout,
      const Swapchain& swapChain, const TextureImage& textureImage,
      const std::vector<VkDescriptorBufferInfo> bufferInfos);

  void createDescriptorPool(const DeviceManager& deviceManager,
                            const Swapchain& swapChain);

  void createGraphicsDescriptorSets(
      const DeviceManager& deviceManager,
      const DescriptorSetLayout& descriptorSetLayout,
      const TextureImage& textureImage, const TextureSampler& textureSampler);

  const uint32_t BUFFER_DESCRIPTORS = 2;
  VkDescriptorPool descriptorPool;
  VkDescriptorSet descriptorSet;
};
}  // namespace odin
#endif  // ODIN_DESCRIPTOR_POOL_HPP