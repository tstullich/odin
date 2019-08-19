#ifndef ODIN_BVH_HPP
#define ODIN_BVH_HPP

#include <glm/glm.hpp>

#include <iostream>
#include <vector>

#include "renderer/aabb.hpp"
#include "renderer/triangle.hpp"

namespace odin {
struct BVH {
  AABB box;
  Triangle left;
  Triangle right;

  void init(float t0, float t1, std::vector<Triangle> &triangles) {
    std::cout << "Building BVH" << std::endl;
    if (triangles.size() < 1) {
      std::cout << "No triangles parsed. Returning empty structure."
                << std::endl;
      return;
    }
    AABB temp_box;
    bool first_true = triangles[0].calculateAABB(t0, t1, temp_box);
    if (!first_true) {
      return;
    } else {
      box = temp_box;
    }

    for (int i = 1; i < triangles.size(); i++) {
      if (triangles[0].calculateAABB(t0, t1, temp_box)) {
        box = AABB::surroundingBox(box, temp_box);
      }
    }
  }

  bool boundingBox(float t0, float t1, AABB &b) {
    b = box;
    return true;
  }
};
} // namespace odin
#endif // ODIN_BVH_HPP