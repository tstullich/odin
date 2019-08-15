#ifndef ODIN_CAMERA_HPP
#define ODIN_CAMERA_HPP

#include <glm/glm.hpp>

namespace odin {
struct Camera {
  glm::vec3 origin;
  glm::vec3 lower_left_corner;
  glm::vec3 horizontal;
  glm::vec3 vertical;

  void init() {
    lower_left_corner = glm::vec3(-2.0f, -1.0f, -1.0f);
    // Setting horizontal and vertical according to the final
    // aspect ratio of the image (800x600 = 4:3)
    horizontal = glm::vec3(4.0f, 0.0f, 0.0f);
    vertical = glm::vec3(0.0f, 3.0f, 0.0f);
    origin = glm::vec3(0.0f, 0.0f, 0.0f);
  }
};

extern Camera camera;
}  // namespace odin
#endif  // ODIN_CAMERA_HPP