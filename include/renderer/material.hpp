#ifndef ODIN_MATERIAL_HPP
#define ODIN_MATERIAL_HPP

#include <glm/glm.hpp>

namespace odin {
struct Material {
  alignas(16) glm::vec3 albedo;
  alignas(16) float fuzz;            // Used for metals microfacets
  alignas(16) float ref_idx;         // Used for refraction in dielectrics
  alignas(16) int scatter_function;  // 1 = diffuse/lambertian 2 = metal 3 =
                                     // dielectric
};
}  // namespace odin
#endif  // ODIN_MATERIAL_HPP