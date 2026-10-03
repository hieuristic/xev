#pragma once
#include <xev/pipeline/pipeline.h>

namespace xev {
struct Scene;
}

namespace xev::pipe {

struct Skinning : ComputePipeline {
  struct PushConst {
    VkDeviceAddress jointMats;
    VkDeviceAddress deforms;
    VkDeviceAddress iBuf;
    VkDeviceAddress oBuf;
    uint32_t offset;
    uint32_t maxThread;
  };

  struct DispatchInfo {
  };

  Skinning() {
    info.shaderSrc = "skinning.spv";
    info.pushConstSize = sizeof(Skinning::PushConst);
  }

  void dispatch(VkCommandBuffer cmdbuf, const Scene& scene);
};

}  // namespace xev::pipe
