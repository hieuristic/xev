#pragma once
#include <xev/pipeline/pipeline.h>
#include <span>

namespace xev::pipe {

struct Skinning : ComputePipeline {
  struct PushConst {
    VkDeviceAddress boneTransforms;
    VkDeviceAddress palettes;
    VkDeviceAddress iBuf;
    VkDeviceAddress oBuf;
    uint32_t offset;
    uint32_t maxThread;
  };

  struct DispatchInfo {
    VkDeviceAddress boneTransforms{0};
    VkDeviceAddress palettes{0};
    VkDeviceAddress iBuf{0};
    VkDeviceAddress oBuf{0};
    uint32_t offset{0};
    uint32_t count{0};
  };

  Skinning() {
    info.shaderSrc = "skinning.spv";
    info.pushConstSize = sizeof(Skinning::PushConst);
  }

  // keep in sync with skinning.slang numthreads
  constexpr uint32_t groupSize = 256;

  void dispatch(VkCommandBuffer cmdbuf,
                std::span<const DispatchInfo> infos);
};

}  // namespace xev::pipe
