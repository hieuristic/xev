#pragma once
#include <vector>

namespace xev {

struct Action;

struct Puppet {
  uint32_t id{0};
  uint32_t skeleton_id{0};
  std::vector<uint32_t> mesh_ids;
  std::vector<uint32_t> actions;
};

}  // namespace xev
