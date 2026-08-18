#include <glm/glm.hpp>
#include <xev/camera.h>
#include <xev/geometry/sphere.h>

namespace xev {

struct Frustum {
  Frustum(const Camera& camera);
  ~Frustum();
  bool contains(const Sphere& sphere);

private:
  // corner order in direction of +z, +y, +x (near to far, up to down, left to right)
  // For example: nul, nur, ndl, ndr, ...
  std::array<glm::vec3, 8> m_corners;
  std::array<glm::vec4, 6> m_planes;
};

}  // namespace xev
