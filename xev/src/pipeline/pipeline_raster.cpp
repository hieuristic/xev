#include <xev/logger.h>
#include <xev/pipeline/pipeline_raster.h>

namespace xev {

void PipelineRaster::draw(VkCommandBuffer cmdbuf,
                          VkDeviceAddress infoAddr,
                          uint32_t numDraw,
                          uint32_t width,
                          uint32_t height) {
  vkCmdBindPipeline(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  VkViewport viewport = {
      .x = 0,
      .y = 0,
      .width = static_cast<float>(width),
      .height = static_cast<float>(height),
      .minDepth = 0.f,
      .maxDepth = 1.f,
  };
  vkCmdSetViewport(cmdbuf, 0, 1, &viewport);

  VkRect2D scissor = {
      .offset = {},
      .extent = {width, height},
  };
  vkCmdSetScissor(cmdbuf, 0, 1, &scissor);

  PushConst pushConst{.infoBuffer = infoAddr};
  vkCmdPushConstants(cmdbuf, layout,
                     VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                     0, sizeof(PushConst), &pushConst);

  vkCmdDraw(cmdbuf, 3, numDraw, 0, 0);
}

}  // namespace xev
