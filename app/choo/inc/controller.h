#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace xev {
struct Camera;
}  // namespace xev

namespace ecs::com {
struct Transform;
struct Movement;
}  // namespace ecs::com

struct Controller {
  void update(float dt,
              float mouseRelX,
              float mouseRelY,
              const bool* keys,
              ecs::com::Transform& player_transform,
              ecs::com::Movement& player_movement,
              xev::Camera& camera);

  float yaw{0.0};
  float pitch{0.3};
  float orbitDist{6.0f};
};
