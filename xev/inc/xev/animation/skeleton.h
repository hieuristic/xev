#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <xev/geometry/group.h>

namespace xev {

struct Skeleton {
  uint32_t id{0};
  std::vector<uint16_t> parentIndices;
  std::vector<glm::mat4> invBindMats;
  std::vector<Sim3> pose;
#ifdef XEVDEBUG
  std::vector<std::string> boneNames;
#endif
};

}  // namespace xev
