#pragma once
#include <cstdint>
#include <span>

#include <glm/glm.hpp>

namespace xev {

struct Sim3;

inline constexpr k_NOPARENT = 0xFFFF;

void forward_kinematics(std::span<const Sim3> localTransform,
                        std::span<const uint16_t> parents,
                        std::span<const glm::mat4> invBindMat,
                        std::span<glm::mat4> modelTransform,
                        std::span<glm::mat4> skinning);

}  // namespace xev
