#include <xev/camera.h>

namespace xev {

Camera::Camera() {
  rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  pos = glm::vec3(0.0f);
}

Camera::Camera(glm::quat rot_, glm::vec3 pos_, float fov_deg) {
  rot = rot_;
  pos = pos_;
  fovx_rad = glm::radians(fov_deg);
  fovy_rad = glm::radians(fov_deg);
}

Camera::Camera(glm::quat rot_, glm::vec3 pos_, float fovx_deg, float fovy_deg) {
  rot = rot_;
  pos = pos_;
  fovx_rad = glm::radians(fovx_deg);
  fovy_rad = glm::radians(fovy_deg);
}

Camera::Camera(glm::quat rot_, glm::vec3 pos_, float fovx_deg, float fovy_deg,
               float shift_x_, float shift_y_) {
  rot = rot_;
  pos = pos_;
  fovx_rad = glm::radians(fovx_deg);
  fovy_rad = glm::radians(fovy_deg);
  shift_x = shift_x_;
  shift_y = shift_y_;
}

glm::mat4 Camera::create_view_mat() const {
  glm::mat4 rot4 = glm::mat4_cast(rot);
  return rot4 * glm::translate(glm::mat4(1.0f), -pos);
}

glm::mat4 Camera::create_proj_mat() const {
  float aspect = std::tan(fovx_rad * 0.5f) / std::tan(fovy_rad * 0.5f);
  glm::mat4 proj = glm::perspective(fovy_rad, aspect, near, far);
  proj[2][0] = shift_x;
  proj[2][1] = shift_y;
  return proj;
}

glm::mat4 Camera::create_vp_mat() const {
  glm::mat4 v = create_view_mat();
  glm::mat4 p = create_proj_mat();
  return p * v;
}

} // namespace xev
