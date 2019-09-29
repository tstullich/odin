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
// A struct describing a node in the BVH. The leaf nodes
// will store the actual geometry
struct BvhNode {
  Triangle left;
  Triangle right;
  AABB box;
  alignas(16) bool is_leaf = false;
};

// A struct encapsulating data for a Bounding Volume Hierarchy.
// This should help to accelerate the raytracing done in the compute shader
struct BVH {
  std::vector<BvhNode> nodes;

public:
  void init(std::vector<Triangle> &triangles) {
    if (triangles.size() == 0) {
      throw std::runtime_error("No triangles available to build BVH!");
    }

    buildBVH(triangles, triangles.size());
    // Reverse our vector so we have a proper ordering for the nodes
    std::reverse(nodes.begin(), nodes.end());
  }

private:
  void buildBVH(std::vector<Triangle> &triangles, int n) {
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
    BvhNode node;
    if (n == 1) {
      // If we only have 1 element we copy it into both leaves
      node.left = node.right = triangles[0];
      node.is_leaf = true;
    } else if (n == 2) {
      node.left = triangles[0];
      node.right = triangles[1];
      node.is_leaf = true;
    } else {
      // Recursion to divide triangle list into two search spaces
      buildBVH(triangles, n / 2);

      // Create an iterator for the second half of the triangles array
      auto first = triangles.begin() + (n / 2);
      std::vector<Triangle> rightSide(first, triangles.end());
      buildBVH(rightSide, n - n / 2);
    }

    AABB box_left, box_right;
    if (!node.left.boundingBox(box_left) ||
        !node.right.boundingBox(box_right)) {
      throw std::runtime_error("No bounding box found!");
    }

    // Place constructed node into vector
    node.box = AABB::surroundingBox(box_left, box_right);
    std::cout << "Size of BVH node: " << sizeof(node) << std::endl;
    nodes.push_back(node);
  }
};
} // namespace odin
#endif // ODIN_BVH_HPP