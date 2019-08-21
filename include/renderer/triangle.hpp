#ifndef ODIN_TRIANGLE_HPP
#define ODIN_TRIANGLE_HPP

#include <exception>
#include <glm/glm.hpp>

#include "renderer/aabb.hpp"

namespace odin {
// A struct that describes a triangle composed of three vertices and a normal
// vector
struct Triangle {
  alignas(16) glm::vec3 v0;
  alignas(16) glm::vec3 v1;
  alignas(16) glm::vec3 v2;
  alignas(16) glm::vec3 normal;
  AABB box;

  // We calculate the normal based on a right-handed coordinate system to match
  // the coordinate system convention of Vulkan. The assumption is made that
  // OBJ files match this convention
  void calculateNormal() {
    glm::vec3 u = v1 - v0;
    glm::vec3 v = v2 - v0;
    normal = glm::cross(u, v);
  };

  bool boundingBox(AABB aabb) {
    box = surroundingBox(v0, v1, v2);
    aabb = box;
    return true;
  }

  // Calculate the bounding box based on 3 vertices
  AABB surroundingBox(const glm::vec3 &a, const glm::vec3 &b,
                      const glm::vec3 &c) {
    glm::vec3 small(ffmin(ffmin(a.x, b.x), c.x), ffmin(ffmin(a.y, b.y), c.y),
                    ffmin(ffmin(a.z, b.z), c.z));

    glm::vec3 big(ffmax(ffmax(a.x, b.x), c.x), ffmax(ffmax(a.y, b.y), c.y),
                  ffmax(ffmax(a.z, b.z), c.z));

    return AABB{small, big};
  }
};

// The following structs are used in the BVH construction
// to properly sort the triangles for the BVH
struct CompareX {
  bool operator()(Triangle a, Triangle b) {
    AABB box_left, box_right;
    if (!a.boundingBox(box_left) || !b.boundingBox(box_right)) {
      throw std::runtime_error("No bounding box in BVH node constructor!");
    }
    if (box_left.min.x - box_right.min.x < 0.0f) {
      return -1;
    }
    return 1;
  }
};

struct CompareY {
  bool operator()(Triangle a, Triangle b) {
    AABB box_left, box_right;
    if (!a.boundingBox(box_left) || !b.boundingBox(box_right)) {
      throw std::runtime_error("No bounding box in BVH node constructor!");
    }
    if (box_left.min.y - box_right.min.y < 0.0f) {
      return -1;
    }
    return 1;
  }
};

struct CompareZ {
  bool operator()(Triangle a, Triangle b) {
    AABB box_left, box_right;
    if (!a.boundingBox(box_left) || !b.boundingBox(box_right)) {
      throw std::runtime_error("No bounding box in BVH node constructor!");
    }
    if (box_left.min.z - box_right.min.z < 0.0f) {
      return -1;
    }
    return 1;
  }
};
} // namespace odin
#endif // ODIN_TRIANGLE_HPP