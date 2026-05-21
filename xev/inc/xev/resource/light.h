// should be in sync with src/shaders/light.slang

#pragma once
#include <glm/glm.hpp>

namespace xev {

enum LightType {
  DIRECTIONAL,
  POINT,
  SPOT,
};

class Light {
 public:
  glm::vec3 position;
  glm::vec3 direction;
  glm::vec3 color;

  float range;
  float intensity;
  glm::vec2 offset;
  uint32_t shadowmap_id;
};

}  // namespace xev
