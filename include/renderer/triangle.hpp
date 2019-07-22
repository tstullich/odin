#ifndef ODIN_TRIANGLE_HPP
#define ODIN_TRIANGLE_HPP

#include <glm/glm.hpp>

/**
 * Basic class to store vertex data for a triangle
 */
namespace odin {
struct Triangle {
  // TODO Verify we don't need to mess with memory alignment
  glm::vec3 v0;
  glm::vec3 v1;
  glm::vec3 v2;
  glm::vec3 normal;
};
};  // namespace odin
#endif  // ODIN_TRIANGLE_HPP