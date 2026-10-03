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

  float m_yaw{0.0f};
  float m_pitch{-0.3f};
  float m_distance{6.0f};
  glm::vec3 m_targetOffset{0.0f, -1.2f, 0.0f};
  float m_sensitivity{0.05f};
};
