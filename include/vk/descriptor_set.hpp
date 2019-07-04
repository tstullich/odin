#ifndef ODIN_DESCRIPTOR_SET_HPP
#define ODIN_DESCRIPTOR_SET_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <iostream>

namespace odin {
class DescriptorSet {
 public:
  DescriptorSet(const VkDevice& logicalDevice);
  ~DescriptorSet();

  const VkDescriptorSetLayout getDescriptorSetLayout() const;

 private:
  void createDescriptorSetLayout(const VkDevice& logicalDevice);

  VkDescriptorSetLayout descriptorSetLayout;
};
}  // namespace odin

#endif  // ODIN_DESCRIPTOR_SET_HPP