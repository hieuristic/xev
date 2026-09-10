#pragma once
#include <cstdint>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace xev {
struct Scene;
struct Camera;
}  // namespace xev

namespace ecs {
namespace com {

struct Transform {
  glm::vec2 pos{0.0f};
  glm::quat rot{1.0f, 0.0f, 0.0f, 0.0f};
};

struct Movement {
  glm::vec3 vel{0.0};
  float speed;
};

struct Camera {
  float dist;
};

struct Mesh {
  entt::entity owner{entt::null};
  uint32_t mesh_index;
};

}  // namespace com

namespace sys {

void init(entt& );
void transform();
void movement();
void camera();
void render();

}  // namespace sys
}  // namespace ecs
