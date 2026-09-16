#pragma once
#include <cstdint>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace xev {
struct Scene;
struct Camera;
}  // namespace xev

enum struct CharacterType : uint8_t;

namespace ecs {
namespace com {

struct Transform {
  glm::vec3 pos{0.0f};
  glm::quat rot{1.0f, 0.0f, 0.0f, 0.0f};
  glm::mat4 world_mat{1.0f};
};

struct Movement {
  glm::vec3 velocity{0.0};
  float speed{5.0f};
};

struct Mesh {
  entt::entity owner{entt::null};
  uint32_t meshIdx{0};
  glm::mat4 localOffset{1.0f};
};

struct Player {};
struct Player2 {};

}  // namespace com

namespace sys {

void init(entt::registry& registry,
          xev::Scene& scene,
          entt::entity& player,
          entt::entity& player2,
          entt::entity& map,
          CharacterType& character);
void movement(entt::registry& registry, float dt, const bool* keys);
void transform(entt::registry& registry);
void render_sync(entt::registry& registry, xev::Scene& scene);

}  // namespace sys
}  // namespace ecs
