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
  std::string name;
  glm::vec3 position{0.0f, 0.0f, 0.0f};
  glm::vec3 direction{0.0f, 0.0f, 1.0f};
  glm::vec3 color{1.0f, 1.0f, 1.0f};

  float range{5.0f};
  float intensity{1.0f};
  glm::vec2 offset{0.0f, 0.0f};
  uint32_t shadowmap_id{0};
};

}  // namespace xev
