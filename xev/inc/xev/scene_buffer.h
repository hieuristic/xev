// This is in-sync with scene.slang
#pragma once
#include <glm/glm.hpp>

namespace xev {

struct SceneBuffer {
  // ambient
  glm::vec3 ambient_color{1.0f};
  float ambient_intensity{0.05f};
  float exposure{0.0016f};

  // fog
  glm::vec3 fog_color;
  float fog_density;

  // light
  VkDeviceAddress light_buffer_address;
  uint32_t num_lights;

  // material
  VkDeviceAddress material_buffer_address;
};

}  // namespace xev
