#include "vk/ubo.hpp"
#include <glm/glm.hpp>

namespace odin {
struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};
}  // namespace odin