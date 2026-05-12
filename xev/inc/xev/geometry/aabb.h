#pragma once
#include <glm/glm.hpp>

namespace xev {

class AABB {
public:
  AABB() = default;
  AABB(glm::vec3 pmin_, glm::vec3 pmax_) : pmin(pmin_), pmax(pmax_) {};
  uint32_t volume();
  uint32_t diagonal_length();
  uint32_t shortest_length();
  uint32_t longest_length();
  bool is_empty();

  glm::vec3 pmin;
  glm::vec3 pmax;
};

}
