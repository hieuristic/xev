#pragma once
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

#include <xev/geomtry/group.h>

namespace xev {

struct Bone {
  Sim3 T;
#ifdef XEVDEBUG
  std::string name;
#endif
};

struct Skeleton {
  std::vector<uint32_t> parentIndices;
  std::vector<glm::mat4> invBindMats;
  std::vector<Bone> bones;
};

}  // namespace xev
