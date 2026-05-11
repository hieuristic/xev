#pragma once
#include <glm/glm.hpp>

namespace xev {

class Sphere {
 public:
  Sphere(glm::vec3 origin_, float radius_);
  float sdf(glm::vec3 query);
  float radius;
  glm::vec3 origin;
};

}  // namespace xev
