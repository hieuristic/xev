#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace xev {

struct Sim3 {
  glm::quat r{1.0f, 0.0f, 0.0f, 0.0f};  // rotation
  glm::vec3 p{0.0f};                    // position
  glm::vec3 s{1.0f};                    // scale

  inline glm::mat4 as_mat() const {
    const glm::mat3 R = glm::mat3_cast(r);
    return glm::mat4(glm::vec4(R[0] * s.x, 0.0f), glm::vec4(R[1] * s.y, 0.0f),
                     glm::vec4(R[2] * s.z, 0.0f), glm::vec4(p, 1.0f));
  }
};

struct SE3 {
  glm::quat r;  // rotation
  glm::vec3 p;  // position
};

}  // namespace xev
