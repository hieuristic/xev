#include <xev/geometry/aabb.h>

namespace xev {
float AABB::volume() {
  return (pmax.x - pmin.x) * (pmax.y - pmin.y) * (pmax.z - pmin.z);
}

float AABB::diagonal_length() {
  return sqrt((pmax.x - pmin.x) * (pmax.x - pmin.x) +
              (pmax.y - pmin.y) * (pmax.y - pmin.y) +
              (pmax.z - pmin.z) * (pmax.z - pmin.z));
}

float AABB::shortest_length() {
  return fmin(fmin((pmax.x - pmin.x), (pmax.y - pmin.y)), (pmax.z - pmin.z));
}

float AABB::longest_length() {
  return fmax(fmax((pmax.x - pmin.x), (pmax.y - pmin.y)), (pmax.z - pmin.z));
}

bool AABB::is_empty() {
  return (((pmax.x - pmin.x) * (pmax.y - pmin.y) * (pmax.z - pmin.z)) == 0.0f);
}

}  // namespace xev
