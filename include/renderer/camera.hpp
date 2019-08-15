#ifndef ODIN_CAMERA_HPP
#define ODIN_CAMERA_HPP

#include <glm/glm.hpp>

namespace odin {
struct Camera {
  glm::vec3 origin;
  glm::vec3 lower_left_corner;
  glm::vec3 horizontal;
  glm::vec3 vertical;

  static void init(float vFov, float aspect) {
    float theta = vFov * M_PI / 180;
    float half_height = tan(theta / 2);
    float half_width = aspect * half_height;
    lower_left_corner = glm::vec3(-half_width, -half_height, -1.0f);
    horizontal = glm::vec3(2 * half_width, 0.0f, 0.0f);
    vertical = glm::vec3(0.0f, 2 * half_height, 0.0f);
    origin = vec3(0.0f, 0.0f, 0.0f);
  }
};
}  // namespace odin
#endif  // ODIN_CAMERA_HPP