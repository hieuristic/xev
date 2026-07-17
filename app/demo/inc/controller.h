#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

class Controller {
 public:
  Controller() = default;
  SDL_Scancode k_front = SDL_SCANCODE_W;
  SDL_Scancode k_back = SDL_SCANCODE_S;
  SDL_Scancode k_right = SDL_SCANCODE_A;
  SDL_Scancode k_left = SDL_SCANCODE_D;

  void update(float dt,
              float xrel,
              float yrel,
              const bool* key,
              glm::vec3& pos,
              glm::quat& rot);

 private:
  float m_velocity_coef = 100.0;
  float m_sensitivity = 0.5;
};
