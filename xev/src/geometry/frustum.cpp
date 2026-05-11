#include <xev/geometry/frustum.h>

namespace xev {

Frustum::Frustum(const Camera& camera) {
  float tan_half_x = glm::tan(camera.fovx_rad * 0.5f);
  float tan_half_y = glm::tan(camera.fovy_rad * 0.5f);

  // Half-extents at each plane
  float nx = camera.near * tan_half_x;
  float ny = camera.near * tan_half_y;
  float nz = camera.near;
  float fx = camera.far * tan_half_x;
  float fy = camera.far * tan_half_y;
  float fz = camera.far;

  // Corners in local coordinate (RDF convention)
  m_corners = {{
      {-nx, -ny, nz},  // near top left
      {nx, -ny, nz},   // near top right
      {-nx, ny, nz},   // near bottom left
      {nx, ny, nz},    // near bottom right
      {-fx, -fy, fz},  // far top left
      {fx, -fy, fz},   // far top right
      {-fx, fy, fz},   // far bottom left
      {fx, fy, fz},    // far bottom right
  }};

  // transform corners to world coordinate
  for (auto& c : m_corners) {
    c = camera.rot * c + camera.pos;
  }

  glm::mat3 rot = glm::mat3_cast(camera.rot);
  glm::vec3 right = rot[0];
  glm::vec3 down = rot[1];
  glm::vec3 forward = rot[2];
  glm::vec3 up = -down;

  glm::vec3 n_near = forward;
  glm::vec3 n_far = -forward;
  glm::vec3 n_right = glm::normalize(glm::cross(up, forward + right * tan_half_x));
  glm::vec3 n_left = glm::normalize(glm::cross(forward - right * tan_half_x, up));
  glm::vec3 n_top = glm::normalize(glm::cross(right, forward - down * tan_half_y));
  glm::vec3 n_bot = glm::normalize(glm::cross(forward + down * tan_half_y, right));

  auto make_plane = [](const glm::vec3& n, const glm::vec3& p) -> glm::vec4 {
    return {n.x, n.y, n.z, -glm::dot(n, p)};
  };

  m_planes[0] = make_plane(n_near, camera.pos + forward * camera.near);
  m_planes[1] = make_plane(n_far, camera.pos + forward * camera.far);
  m_planes[2] = make_plane(n_right, camera.pos);
  m_planes[3] = make_plane(n_left, camera.pos);
  m_planes[4] = make_plane(n_top, camera.pos);
  m_planes[5] = make_plane(n_bot, camera.pos);
}

Frustum::~Frustum() {}

bool Frustum::contains(const Sphere& sphere) {
  for (const auto& plane : m_planes) {
    float d = glm::dot(glm::vec3(plane), sphere.origin) + plane.w;
    if (d < -sphere.radius)
      return true;
  }
  return false;
}

}  // namespace xev
