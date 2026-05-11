#include <xev/geometry/sphere.h>

namespace xev {
Sphere::Sphere(glm::vec3 origin_, float radius_) {
  origin = origin_;
  radius = radius_;
}
float Sphere::sdf(glm::vec3 query) {
  return glm::dot(query, origin) - radius;
}

}  // namespace xev
