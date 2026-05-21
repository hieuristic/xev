#include <controller.h>

void Controller::update(float dt,
                        float xrel,
                        float yrel,
                        const bool* key,
                        glm::vec3& pos,
                        glm::quat& rot) {
  float speed = dt * m_velocity_coef;
  glm::mat4 R = glm::mat4_cast(rot);
  glm::vec3 forward = glm::vec3(R[2]);
  glm::vec3 right = glm::vec3(R[0]);
  glm::vec3 up = -glm::vec3(R[1]);
  if (key[k_front])
    pos += forward * speed;
  if (key[k_back])
    pos -= forward * speed;
  if (key[k_right])
    pos -= right * speed;
  if (key[k_left])
    pos += right * speed;

  // // Q and E for up/down (world space)
  // if (m_keystate[SDL_SCANCODE_Q])
  //   .pos -= glm::vec3(0.0f, 1.0f, 0.0f) * speed;  // Up (negative Y)
  // if (m_keystate[SDL_SCANCODE_E])
  //   .pos += glm::vec3(0.0f, 1.0f, 0.0f) * speed;  // Down (positive Y)

  float ddeg = dt * m_sensitivity;
  glm::quat q_yaw = glm::angleAxis(-xrel * ddeg, up);
  glm::quat q_pitch = glm::angleAxis(-yrel * ddeg, right);

  // Apply: yaw first, then pitch
  rot = q_yaw * rot;
  rot = q_pitch * rot;
  rot = glm::normalize(rot);
}
