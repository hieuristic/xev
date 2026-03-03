#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace xev {

enum CAMERA_TYPE {
  CAM_PERSPECTIVE,
  CAM_OTHOGRAPHIC,
};

class Camera {
public:
  Camera();
  Camera(glm::quat rot_, glm::vec3 pos_, float fov_deg);
  Camera(glm::quat rot_, glm::vec3 pos_, float fovx_deg, float fovy_deg);
  Camera(glm::quat rot_, glm::vec3 pos_, float fovx_deg, float fovy_deg,
         float shift_x_, float shift_y_);

  glm::mat4 create_view_mat() const;
  glm::mat4 create_proj_mat() const;
  glm::mat4 create_vp_mat() const;

  glm::quat rot;
  glm::vec3 pos;
  float fovx_rad = glm::radians(70.0);
  float fovy_rad = glm::radians(70.0);
  float shift_x = 0.0;
  float shift_y = 0.0;
  float near = 0.0;
  float far = 1000.0;
};

} // namespace xev
