#ifndef ODIN_AABB_HPP
#define ODIN_AABB_HPP

#include <glm/glm.hpp>

namespace odin {
inline float ffmin(float a, float b) { return a < b ? a : b; }

inline float ffmax(float a, float b) { return a > b ? a : b; }

// A struct that describes an Axis-Aligned Bounding Box
struct AABB {
  alignas(16) glm::vec3 min;
  alignas(16) glm::vec3 max;

  void init(const glm::vec3 &a, const glm::vec3 &b) {
    min = a;
    max = b;
  }

  static AABB surroundingBox(AABB box0, AABB box1) {
    glm::vec3 small(ffmin(box0.min.x, box1.min.x),
                    ffmin(box0.min.y, box1.min.y),
                    ffmin(box0.min.z, box1.min.z));

    glm::vec3 big(ffmax(box0.max.x, box1.max.x), ffmax(box0.max.y, box1.max.y),
                  ffmax(box0.max.z, box1.max.z));

    return AABB{small, big};
  }
};
} // namespace odin
#endif // ODIN_AABB_HPP