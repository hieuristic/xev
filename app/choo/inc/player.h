#pragma once

#include <glm/glm.hpp>

#include <xev/geometry/bound.h>

struct Player {
  glm::vec3 position;
  glm::quat rotation;
  Bound3 hitBox;
};
