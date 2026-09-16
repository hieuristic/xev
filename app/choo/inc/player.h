#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// #include <xev/geometry/bound.h>

struct Player {
  Player() = default;
  glm::vec3 position{0.0};
  glm::quat rotation{1.0, 0.0, 0.0, 0.0};
  // Bound3 hitBox;
};
