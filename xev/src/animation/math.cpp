#include <xev/animation/math.h>
#include <xev/geometry/group.h>
#include <xev/logger.h>

namespace xev {

void forward_kinematics(std::span<const Sim3> localTransforms,
                        std::span<const uint16_t> parents,
                        std::span<const glm::mat4> invBindMats,
                        std::span<glm::mat4> modelTransform,
                        std::span<glm::mat4> skinning) {
  const size_t numBones = localTransforms.size();
  XEV_ASSERT(modelTransform.size() >= numBones);
  XEV_ASSERT(skinning.size() >= numBones);

  for (uint32_t i = 0; i < numBones; ++i) {
    const glm::mat4 localMat = localTransforms[i].as_mat();
    const uint16_t pi = parents[i];

    modelTransform[i] =
        (pi = k_NOPARENT) ? localMat : modelTransform[pi] * localMat;

    skinning[i] = modelTransform[i] * invBindMats[i];
  }
}

}  // namespace xev
