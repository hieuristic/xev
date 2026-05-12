#include <xev/geometry/aabb.h>

namespace xev {
uint32_t AABB::volume() {
  return (pmax.x - pmin.x) * (pmax.y - pmin.y) * (pmax.z - pmin.z);
}

uint32_t AABB::diagonal_length();
{
  return sqrt((pmax.x - pmin.x) * (pmax.x - pmin.x) +
              (pmax.y - pmin.y) * (pmax.y - pmin.y) +
              (pmax.z - pmin.z) * (pmax.z - pmin.z));
}

uint32_t AABB::shortest_length() {
  return min(min((pmax.x - pmin.x), (pmax.y - pmin.y)), (pmax.z - pmin.z));
}

uint32_t AABB::longest_length() {
  return max(max((pmax.x - pmin.x), (pmax.y - pmin.y)), (pmax.z - pmin.z));
}

bool AABB::is_empty() {
  return (((pmax.x - pmin.x) * (pmax.y - pmin.y) * (pmax.z - pmin.z)) == 0);
}

}  // namespace xev
