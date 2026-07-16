// should be in sync with src/shaders/light.slang

#pragma once
#include <glm/glm.hpp>

namespace xev {

enum LightType {
  DIRECTIONAL,
  POINT,
  SPOT,
};

// in-sync with /shaders/light.slang
struct alignas(16) LightGPU {
  uint32_t type;
  float pad0[3];
  glm::vec3 position;
  float pad1;
  glm::vec3 direction;
  float pad2;
  glm::vec3 color;

  float range;
  float intensity;
  glm::vec2 offset;
  uint32_t shadowmap_id;
};

class Light {
 public:
  std::string name;
  LightType type{LightType::POINT};
  glm::vec3 position{0.0f, 0.0f, 0.0f};
  glm::vec3 direction{0.0f, 0.0f, 1.0f};
  glm::vec3 color{1.0f, 1.0f, 1.0f};

  float range{5.0f};
  float intensity{1.0f};
  glm::vec2 offset{0.0f, 0.0f};
  uint32_t shadowmap_id{0};
};

}  // namespace xev
