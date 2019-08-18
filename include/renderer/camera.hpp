#ifndef ODIN_CAMERA_HPP
#define ODIN_CAMERA_HPP

#include <glm/glm.hpp>

namespace odin {
struct Camera {
  alignas(16) glm::vec3 origin;
  alignas(16) glm::vec3 lower_left_corner;
  alignas(16) glm::vec3 horizontal;
  alignas(16) glm::vec3 vertical;
  alignas(16) glm::vec3 u;
  alignas(16) glm::vec3 v;
  alignas(16) glm::vec3 w;
  alignas(16) float lens_radius;

  void init(glm::vec3 lookFrom, glm::vec3 lookAt, glm::vec3 vUp, float vfov,
            float aspect, float aperture, float focusDist) {
    lens_radius = aperture / 2.0f;
    glm::vec3 u, v, w;
    float theta = vfov * M_PI / 180;
    float half_height = glm::tan(theta / 2);
    float half_width = aspect * half_height;
    origin = lookFrom;
    w = glm::normalize(lookFrom - lookAt);
    u = glm::normalize(glm::cross(vUp, w));
    v = glm::cross(w, u);
    lower_left_corner = origin - half_width * focusDist * u -
                        half_height * focusDist * v - focusDist * w;
    horizontal = 2.0f * half_width * focusDist * u;
    vertical = 2.0f * half_height * focusDist * v;
  }
};
} // namespace odin
#endif // ODIN_CAMERA_HPP