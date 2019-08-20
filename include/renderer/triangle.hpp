#ifndef ODIN_TRIANGLE_HPP
#define ODIN_TRIANGLE_HPP

#include <glm/glm.hpp>
#include <exception>

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
    normal = glm::normalize(glm::cross(u, v));
  };

  bool boundingBox(float t0, float t1, AABB aabb) {
    // TODO Implement
    box = aabb;
    return true;
  }
};

// The following structs are used in the BVH construction
// to properly sort the triangles for the BVH
struct CompareX {
  bool operator()(Triangle a, Triangle b) {
    AABB box_left, box_right;
    if (!a.boundingBox(0.0f, 0.0f, box_left) ||
        !b.boundingBox(0.0f, 0.0f, box_right)) {
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
    if (!a.boundingBox(0.0f, 0.0f, box_left) ||
        !b.boundingBox(0.0f, 0.0f, box_right)) {
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
    if (!a.boundingBox(0.0f, 0.0f, box_left) ||
        !b.boundingBox(0.0f, 0.0f, box_right)) {
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