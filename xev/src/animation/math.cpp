#include <xev/animation/math.h>
#include <xev/geometry/group.h>
#include <xev/logger.h>

namespace xev {

void forward_kinematics(std::span<const Sim3> localPoses,
                        std::span<const uint16_t> parents,
                        std::span<const glm::mat4> invBindMats,
                        std::span<Sim3> modelPoses) {
  const size_t numBones = localPoses.size()
  XEV_ASSERT(parents.size() >= numBones);
  XEV_ASSERT(invBindMats.size() >= numBones);


}

}
