#ifndef ODIN_BVH_HPP
#define ODIN_BVH_HPP

#include <glm/glm.hpp>

#include <algorithm>
#include <exception>
#include <random>
#include <vector>

#include "renderer/aabb.hpp"
#include "renderer/triangle.hpp"

namespace odin {

struct BVH_Node {
  Triangle left;
  Triangle right;
  AABB box;
};

struct BVH {
  BVH_Node root;

public:
  void init(std::vector<Triangle> &triangles) {
    root = buildBVH(triangles, triangles.size(), 0.0, 1.0);
  }

private:
  BVH_Node buildBVH(const std::vector<Triangle> &triangles, int n, float t0,
                    float t1) {
    // Setup random number generator
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 2);

    // Choose which axis to sort over
    int axis = dist(rng);
    if (axis == 0) {
      // Sort over x axis
      std::sort(triangles.begin(), triangles.end(), CompareX());
    } else if (axis == 1) {
      // Sort over y axis
      std::sort(triangles.begin(), triangles.end(), CompareY());
    } else {
      // Sort over z axis
      std::sort(triangles.begin(), triangles.end(), CompareZ());
    }

    // Decide where to place the geometry into the leaf node
    BVH_Node node;
    if (n == 1) {
      // If we only have 1 element we copy it into both leaves
      node.left = node.right = triangles[0];
    } else if (n == 2) {
      node.left = triangles[0];
      node.right = triangles[1];
    } else {
      // Recursion to divide triangle list into two search spaces
      node = buildBVH(triangles, n / 2, t0, t1);

      // Create an iterator for the second half of the triangles array
      auto first = triangles.begin() + (n / 2);
      std::vector<Triangle> rightSide(first, triangles.end());
      node = buildBVH(rightSide, n - n / 2, t0, t1);
    }

    AABB box_left, box_right;
    if (!node.left.boundingBox(t0, t1, box_left) ||
        !node.right.boundingBox(t0, t1, box_right)) {
      throw std::runtime_error("No bounding box found!");
    }

    node.box = AABB::surroundingBox(box_left, box_right);
    return node;
  }
};
} // namespace odin
#endif // ODIN_BVH_HPP