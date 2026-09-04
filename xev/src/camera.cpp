#include <cmath>

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

Camera::Camera(glm::quat rot_,
               glm::vec3 pos_,
               float fovx_deg,
               float fovy_deg,
               float shift_x_,
               float shift_y_) {
  rot = rot_;
  pos = pos_;
  fovx_rad = glm::radians(fovx_deg);
  fovy_rad = glm::radians(fovy_deg);
  shift_x = shift_x_;
  shift_y = shift_y_;
}

glm::mat4 Camera::create_view_mat() const {
  glm::mat4 rot_inv = glm::mat4_cast(glm::conjugate(rot));
  glm::mat4 trans_inv = glm::translate(glm::mat4(1.0f), -pos);
  return rot_inv * trans_inv;
}

glm::mat4 Camera::create_proj_mat() const {
  float aspect = std::tan(fovx_rad * 0.5f) / std::tan(fovy_rad * 0.5f);
  // For RDF (Right-Handed, +Z is Forward, +Y is Down, +X is Right)
  // Vulkan clip space: +X Right, +Y Down, +Z Forward (0 to 1)
  glm::mat4 proj = glm::mat4(0.0f);
  proj[0][0] = 1.0f / aspect / std::tan(fovy_rad * 0.5f);
  proj[1][1] = 1.0f / std::tan(fovy_rad * 0.5f);
  proj[2][2] = far / (far - near);
  proj[2][3] = 1.0f;
  proj[3][2] = -(far * near) / (far - near);
  return proj;
}

glm::mat4 Camera::create_vp_mat() const {
  glm::mat4 v = create_view_mat();
  glm::mat4 p = create_proj_mat();
  return p * v;
}

void Camera::set_aspect(float aspect_) {
  aspect = aspect_;
  fovx_rad = 2.0f * std::atan(aspect * std::tan(fovy_rad * 0.5f));
}

void Camera::set_fov_deg(float deg) {
  fovy_rad = glm::radians(deg);
  fovx_rad = 2.0f * std::atan(aspect * std::tan(fovy_rad * 0.5f));
}

float Camera::ev100() const {
  return std::log2((aperture * aperture * shutter_freq) * (100.0f / iso));
}

float Camera::exposure() const {
  return 1.0f / (1.2f * std::pow(2.0f, ev100()));
}

}  // namespace xev
