#ifndef ODIN_AABB_HPP
#define ODIN_AABB_HPP

#include <glm/glm.hpp>

namespace odin {
// Inlining these two functions which have less error checks
// than the standard fmin/fmax
inline float ffmin(float a, float b) { return a < b ? a : b; }
inline float ffmax(float a, float b) { return a > b ? a : b; }

// A struct that describes an Axis-Aligned Bounding Box
struct AABB {
  alignas(16) glm::vec3 min;
  alignas(16) glm::vec3 max;

  // Build a surrounding box from two AABBs
  static AABB surroundingBox(AABB box0, AABB box1) {
    glm::vec3 min(ffmin(box0.min.x, box1.min.x), ffmin(box0.min.y, box1.min.y),
                  ffmin(box0.min.z, box1.min.z));

    glm::vec3 max(ffmax(box0.max.x, box1.max.x), ffmax(box0.max.y, box1.max.y),
                  ffmax(box0.max.z, box1.max.z));

    return AABB{min, max};
  }
};
} // namespace odin
#endif // ODIN_AABB_HPP