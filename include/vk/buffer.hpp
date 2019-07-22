#ifndef ODIN_BUFFER_HPP
#define ODIN_BUFFER_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>

#include "vk/command_pool.hpp"
#include "vk/device_manager.hpp"

namespace odin {
// Forward declarations
class CommandPool;

class Buffer {
 public:
  static uint32_t findMemoryType(const VkPhysicalDevice& physicalDevice,
                                 uint32_t typeFilter,
                                 VkMemoryPropertyFlags properties);

  static void createBuffer(const DeviceManager& deviceManager,
                           VkDeviceSize size, VkBufferUsageFlags usageFlags,
                           VkMemoryPropertyFlags properties, VkBuffer& buffer,
                           VkDeviceMemory& bufferMemory);

 protected:
  Buffer();

  ~Buffer();

  void copyBuffer(const DeviceManager& deviceManager,
                  const CommandPool& commandPool, VkBuffer srcBuffer,
                  VkBuffer dstBuffer, VkDeviceSize size);

  void createBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice,
                    VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, VkBuffer& buffer,
                    VkDeviceMemory& bufferMemory);

  VkBuffer buffer;
};
}  // namespace odin
#endif  // ODIN_BUFFER_HPP