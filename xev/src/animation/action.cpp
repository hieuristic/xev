#include <cmath>

#include <xev/animation/action.h>
#include <xev/animation/interp.h>
#include <xev/logger.h>

namespace xev {

void Action::sample(float T, std::span<Sim3> samplePose) const {
  XEV_ASSERT(samplePose.size() >= numBones);
  XEV_ASSERT(track.size() == size_t(numFrames) * numBones);
  if (numFrames == 0 || track.empty()) return;

  float t = T * fps;
  t = std::fmod(t, float(numFrames));
  if (t < 0.0) t += float(numFrames);

  uint32_t t0 = static_cast<uint32_t>(t);
  if (t0 >= numFrames) t0 = numFrames - 1;
  uint32_t t1 = t0 + 1;
  if (t1 == numFrames) t1 = 0;
  float w = t - static_cast<float>(t0);

  const Sim3* p0 = track.data() + t0 * numBones;
  const Sim3* p1 = track.data() + t1 * numBones;

  for (uint32_t i = 0; i < numBones; ++i) {
    samplePose[i].r = nlerp(p0[i].r, p1[i].r, w);
    samplePose[i].p = glm::mix(p0[i].p, p1[i].p, w);
    samplePose[i].s = glm::mix(p0[i].s, p1[i].s, w);
  }
}

}  // namespace xev
