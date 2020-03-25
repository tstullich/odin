#ifndef ODIN_RENDER_PASS_HPP
#define ODIN_RENDER_PASS_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <iostream>
#include <stdexcept>

namespace odin {
class RenderPass {
 public:
  RenderPass(const VkDevice& logicalDevice, const VkFormat& imageFormat,
             const VkFormat& depthFormat);

  const VkRenderPass getRenderPass() const;

 private:
  void createRenderPass(const VkDevice& logicalDevice,
                        const VkFormat& imageFormat,
                        const VkFormat& depthFormat);

  VkRenderPass renderPass;
};
}  // namespace odin
#endif  // ODIN_RENDER_PASS