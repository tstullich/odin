#ifndef ODIN_TEXTURE_SAMPLER_HPP
#define ODIN_TEXTURE_SAMPLER_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "vk/device_manager.hpp"

namespace odin {
class TextureSampler {
 public:
  TextureSampler(const DeviceManager& deviceManager);

  ~TextureSampler();

  const VkSampler getSampler() const;

 private:
  VkSampler textureSampler;
};
}  // namespace odin
#endif  // ODIN_TEXTURE_SAMPLER_HPP