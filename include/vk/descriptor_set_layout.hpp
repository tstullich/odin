#ifndef ODIN_DESCRIPTOR_SET_LAYOUT_HPP
#define ODIN_DESCRIPTOR_SET_LAYOUT_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <iostream>
#include <stdexcept>

#include "vk/device_manager.hpp"

namespace odin {
class DescriptorSetLayout {
 public:
  DescriptorSetLayout(const DeviceManager& deviceManager,
                      bool computeDescriptorSet);

  const VkDescriptorSetLayout* getDescriptorSetLayout() const;

 private:
  void createComputeDescriptorSetLayout(const DeviceManager& deviceManager);

  void createGraphicsDescriptorSetLayout(const DeviceManager& deviceManager);

  VkDescriptorSetLayout descriptorSetLayout;
};
}  // namespace odin
#endif  // ODIN_DESCRIPTOR_SET_LAYOUT_HPP