#pragma once
#include <xev/pipeline/pipeline.h>
#include <span>

namespace xev::pipe {

struct Skinning : ComputePipeline {
  struct PushConst {
    VkDeviceAddress boneTransforms;
    VkDeviceAddress boneIdices{0};
    VkDeviceAddress boneWeights{0};
    VkDeviceAddress iBuf;
    VkDeviceAddress oBuf;
    uint32_t offset;
    uint32_t maxThread;
  };

  struct DispatchInfo {
    VkDeviceAddress boneTransforms{0};
    VkDeviceAddress boneIdices{0};
    VkDeviceAddress boneWeights{0};
    VkDeviceAddress iVertBuf{0};
    VkDeviceAddress oVertBuf{0};
    uint32_t offset{0};
    uint32_t count{0};
  };

  Skinning() {
    info.shaderSrc = "skinning.spv";
    info.pushConstSize = sizeof(Skinning::PushConst);
  }

  // keep in sync with skinning.slang numthreads
  constexpr uint32_t groupSize = 256;

  void dispatch(VkCommandBuffer cmdbuf, std::span<const DispatchInfo> infos);
};

}  // namespace xev::pipe
