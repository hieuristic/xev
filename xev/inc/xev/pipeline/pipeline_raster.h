#pragma once
#include <xev/pipeline/pipeline.h>
#include <xev/volk.h>
#include <glm/glm.hpp>

namespace xev {

class PipelineRaster : public Pipeline {
 public:
  struct PushConst {
    VkDeviceAddress infoBuffer;
  };

  struct DrawInfo {
    glm::mat4 transform;
    glm::vec4 uvBounds;
    uint32_t texID;
    uint32_t isMSDF;
  };

  PipelineRaster() {
      pipeInfo.shaderVertSrc = "raster.spv";
      pipeInfo.shaderFragSrc = "raster.spv";
      pipeInfo.pushConstSize = sizeof(PipelineRaster::PushConst);
      pipeInfo.enableBlending = true;
      pipeInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      pipeInfo.polygonMode = VK_POLYGON_MODE_FILL;
      pipeInfo.cullMode = VK_CULL_MODE_NONE;
      pipeInfo.multisampleCount = VK_SAMPLE_COUNT_1_BIT;
      pipeInfo.enableDepth = false;
  }

  void draw(VkCommandBuffer cmdbuf,
            VkDeviceAddress infoAddr,
            uint32_t numDraw,
            uint32_t width,
            uint32_t height);
};

}  // namespace xev
