#pragma once
#include <cstdint>
#include <vector>

#include <xev/geometry/group.h>

namespace xev {

struct Action {
  float duration;
  float fps;
  uint32_t numBones;
  uint32_t numFrames;

  // flattened bone animation for better caching
  // layout: frame0_bone0, frame0_bone1, ..., frame1_bone0, ...
  std::vector<Sim3> track;

  void sample(float t, std::span<Sim3> samplePose);
};

}  // namespace xev
