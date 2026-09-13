#include <SDL3/SDL.h>
#include <glm/gtc/matrix_transform.hpp>

#include <xev/camera.h>

#include "controller.h"
#include "ecs.h"

void Controller::update(float dt,
                        float mouseRelX,
                        float mouseRelY,
                        const bool* keys,
                        ecs::com::Transform& player_transform,
                        ecs::com::Movement& player_movement,
                        xev::Camera& camera) {
  // mouse control
  m_yaw += mouseRelX * m_sensitivity;
  m_pitch += mouseRelY * m_sensitivity;
  m_pitch = std::clamp(m_pitch, -0.8f, 0.5f);

  // keyboard control
  // RDF (+X right, +Y down, +Z forward):
  glm::vec3 camForward =
      glm::normalize(glm::vec3(sin(m_yaw), 0.0f, cos(m_yaw)));
  glm::vec3 camRight =
      glm::normalize(glm::vec3(camForward.z, 0.0f, -camForward.x));

  // 2. Camera-Relative Movement (Souls-like WASD)
  glm::vec3 moveDir{0.0f};
  if (keys[SDL_SCANCODE_W]) moveDir += camForward;
  if (keys[SDL_SCANCODE_S]) moveDir -= camForward;
  if (keys[SDL_SCANCODE_D]) moveDir += camRight;
  if (keys[SDL_SCANCODE_A]) moveDir -= camRight;

  if (glm::length(moveDir) > 0.001f) {
    moveDir = glm::normalize(moveDir);
    player_movement.velocity = moveDir * player_movement.speed;
    player_transform.pos += player_movement.velocity * dt;

    // immediately rotate the character to face movement direction
    float playerAngle = atan2(moveDir.x, -moveDir.z);
    player_transform.rot =
        glm::angleAxis(playerAngle, glm::vec3(0.0f, -1.0f, 0.0f));
  } else {
    player_movement.velocity = glm::vec3(0.0f);
  }

  // 3. Position Camera on a sphere behind the player's head target
  glm::vec3 target = player_transform.pos + m_targetOffset;
  glm::vec3 offset = glm::vec3(-sin(m_yaw) * cos(m_pitch), sin(m_pitch),
                               -cos(m_yaw) * cos(m_pitch)) *
                     m_distance;

  camera.pos = target + offset;

  // 4. Look at target with RDF up-vector (0, -1, 0)
  glm::vec3 f = glm::normalize(target - camera.pos);
  glm::vec3 r = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), f));
  glm::vec3 d = glm::cross(f, r);
  camera.rot = glm::quat_cast(glm::mat3(r, d, f));
}
