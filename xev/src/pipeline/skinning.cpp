#include <xev/pipeline/skinning.h>
namespace xev {

void pipe::Skinning::dispatch(VkCommandBuffer cmdbuf,
                              std::span<const DispatchInfo> infos) {
  if (infos.count == 0) return;
  vkCmdBindPipeline(cmdbuf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

  for (const auto& d : infos) {
    PushConst pc = {
        .deforms = d.deforms,
        .skinning = d.skinning,
        .iBuf = d.iBuf,
        .oBuf = d.oBuf,
        .offset = d.offset,
        .maxThread = d.count,
    };

    vkCmdPushConstants(cmdbuf, layout, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                       sizeof(PushConst), &pc);
    uint32_t numGroup = (d.count + groupSize - 1) / groupSize;
    vkCmdDispatch(cmdbuf, numGroup, 1, 1);
  }
}

}  // namespace xev
