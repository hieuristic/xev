#pragma once
#include <glm/glm.hpp>

namespace xev {

inline glm::quat nlerp(const glm::quat& a, const glm::quat& b, float t) {
  glm::quat target = (glm::dot(a, b) < 0.0f) ? -b : b;
  return glm::normalize(glm::mix(a, target, t));
}

}  // namespace xev
