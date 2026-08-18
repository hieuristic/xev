#pragma once
#include <glm/glm.hpp>

namespace xev {

struct AABB {
  AABB() = default;
  AABB(glm::vec3 pmin_, glm::vec3 pmax_) : pmin(pmin_), pmax(pmax_) {};
  float volume();
  float diagonal_length();
  float shortest_length();
  float longest_length();
  bool is_empty();

  glm::vec3 pmin;
  glm::vec3 pmax;
};

}
