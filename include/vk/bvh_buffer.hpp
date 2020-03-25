#ifndef ODIN_BVH_BUFFER_HPP
#define ODIN_BVH_BUFFER_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include "renderer/bvh.hpp"
#include "vk/buffer.hpp"
#include "vk/device_manager.hpp"

namespace odin {
// Forward declarations
class CommandPool;

class BvhBuffer : public Buffer {
 public:
  BvhBuffer(const DeviceManager &deviceManager, const CommandPool &commandPool,
            const std::vector<BvhNode> &nodes);

  const VkBuffer getBuffer() const;

  const VkDeviceMemory getBufferMemory() const;

  const VkDescriptorBufferInfo getDescriptor() const;

  const size_t getNodeCount() const;

 private:
  VkDeviceMemory bvhBufferMemory;
  size_t numNodes;
};
}  // namespace odin
#endif  // ODIN_BVH_BUFFER_HPP