#ifndef ODIN_TRIANGLE_HPP
#define ODIN_TRIANGLE_HPP

#include <glm/glm.hpp>

/**
 * Basic class to store vertex data for a triangle
 */
namespace odin {
struct Triangle {
  glm::vec3 v0;
  glm::vec3 v1;
  glm::vec3 v2;
  glm::vec3 normal;

  // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
  void calculateNormal() {
    glm::vec3 u = v1 - v0;
    glm::vec3 v = v2 - v0;

    normal.x = (u.y * v.z) - (u.z - v.y);
    normal.y = (u.z * v.x) - (u.x - v.z);
    normal.z = (u.x * v.y) - (u.y - v.x);
  };
};
}  // namespace odin
#endif  // ODIN_TRIANGLE_HPP