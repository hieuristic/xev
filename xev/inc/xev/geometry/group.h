#pragma once
#include <glm/glm.hpp>

namespace {

struct Sim3 {
  glm::vec3 r; // rotation
  glm::vec3 p; // position
  glm::vec3 s; // scale
};

struct SE3 {
  glm::vec3 r; // rotation
  glm::vec3 p; // position
}

}
