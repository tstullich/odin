#ifndef ODIN_SHADER_MODULE_HPP
#define ODIN_SHADER_MODULE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>

namespace odin {
class ShaderModule {
 public:
  ShaderModule(const VkDevice& logicalDevice,
               const std::vector<char>& shaderCode);

  ~ShaderModule();

  const VkShaderModule getShaderModule() const;

 private:
  VkShaderModule shaderModule;
  VkDevice ownedDevice;
};
}  // namespace odin
#endif  // ODIN_SHADER_MODULE_HPP