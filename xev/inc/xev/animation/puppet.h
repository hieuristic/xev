#pragma once
#include <vector>
#include <xev/animation/skeleton.h>

namespace xev {

struct Action;

struct Puppet {
  uint32_t id{0};
  Skeleton skeleton;
  std::vector<uint32_t> meshes;
  std::vector<uint32_t> actions;
};

}  // namespace xev
