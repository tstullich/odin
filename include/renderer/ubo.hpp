#include <glm/glm.hpp>

namespace odin {
struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

extern UniformBufferObject ubo;
}  // namespace odin