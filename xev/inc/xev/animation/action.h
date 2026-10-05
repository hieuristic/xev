#pragma once
#include <cstdint>
#include <vector>

#include <xev/geometry/group.h>

namespace xev {

struct Action {
  uint32_t id{0};
  float duration{0.0};
  float fps{30.0};
  uint32_t numBones{0};
  uint32_t numFrames{0};

  // flattened bone animation for better caching
  // layout: frame0_bone0, frame0_bone1, ..., frame1_bone0, ...
  std::vector<Sim3> track;

  // t is given in second
  void sample(float T, std::span<Sim3> samplePose) const;
};

}  // namespace xev
