#ifndef ODIN_DESCRIPTOR_POOL_HPP
#define ODIN_DESCRIPTOR_POOL_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "vk/device_manager.hpp"
#include "vk/swapchain.hpp"

namespace odin {
class DescriptorPool {
 public:
  DescriptorPool(const DeviceManager& deviceManager,
                 const Swapchain& swapChain);

  ~DescriptorPool();

  const VkDescriptorPool getDescriptorPool() const;

 private:
  VkDescriptorPool descriptorPool;
};
}  // namespace odin
#endif  // ODIN_DESCRIPTOR_POOL_HPP