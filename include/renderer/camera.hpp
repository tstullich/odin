#ifndef ODIN_CAMERA_HPP
#define ODIN_CAMERA_HPP

#include <glm/glm.hpp>

namespace odin {
struct Camera {
  alignas(16) glm::vec3 origin;
  alignas(16) glm::vec3 lower_left_corner;
  alignas(16) glm::vec3 horizontal;
  alignas(16) glm::vec3 vertical;

  void init() {
    origin = glm::vec3(0.0f, 0.0f, 0.0f);
    lower_left_corner = glm::vec3(-2.0f, -1.0f, -1.0f);
    // Setting horizontal and vertical according to the final
    // aspect ratio of the image (800x600 = 4:3)
    horizontal = glm::vec3(4.0f, 0.0f, 0.0f);
    vertical = glm::vec3(0.0f, 3.0f, 0.0f);
  }
};
}  // namespace odin
#endif  // ODIN_CAMERA_HPP