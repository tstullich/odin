#ifndef ODIN_DESCRIPTOR_POOL_HPP
#define ODIN_DESCRIPTOR_POOL_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "vk/descriptor_set_layout.hpp"
#include "vk/device_manager.hpp"
#include "vk/swapchain.hpp"
#include "vk/texture_image.hpp"
#include "vk/texture_sampler.hpp"
#include "vk/uniform_buffer.hpp"

namespace odin {
class DescriptorPool {
 public:
  DescriptorPool(const DeviceManager& deviceManager, const Swapchain& swapChain,
                 const DescriptorSetLayout& computeDescriptorSetLayout,
                 const DescriptorSetLayout& graphicsDescriptorSetLayout,
                 const std::vector<UniformBuffer>& uniformBuffers,
                 const TextureImage& textureImage,
                 const TextureSampler& textureSampler);

  ~DescriptorPool();

  const VkDescriptorPool getDescriptorPool() const;

  const VkDescriptorSet getDescriptorSet() const;

 private:
  void createComputeDescriptorSets(
      const DeviceManager& deviceManager,
      const DescriptorSetLayout& descriptorSetLayout,
      const Swapchain& swapChain);

  void createDescriptorPool(const DeviceManager& deviceManager,
                            const Swapchain& swapChain);

  void createGraphicsDescriptorSets(
      const DeviceManager& deviceManager,
      const DescriptorSetLayout& descriptorSetLayout,
      const std::vector<UniformBuffer>& uniformBuffers,
      const TextureImage& textureImage, const TextureSampler& textureSampler);

  VkDescriptorPool descriptorPool;
  VkDescriptorSet descriptorSet;
};
}  // namespace odin
#endif  // ODIN_DESCRIPTOR_POOL_HPP