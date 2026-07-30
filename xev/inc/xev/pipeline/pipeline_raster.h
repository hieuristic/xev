#pragma once

namespace xev {

class PipelineRaster : Pipeline {
 public:
  struct PushConst {
    VkDeviceAddress infoBuffer;
  };

  PipelineInfo pipeInfo{
      .shaderVertSrc = "raster.spv",
      .shaderFragSrc = "raster.spv",
      .pushConstSize = sizeof(PipelineRaster::PushConst),
      .enableBlending = true,
      .topology = VK_PRIMITIV_TOPOLOGY_TRIANGLE_LIST,
      .polygonMode = VK_POLYGON_MODE_FULL,
      .cullMode = VK_CULL_MODE_NONE,
      .colorFormat = color_format,
      .multisampleCount = VK_SAMPLE_COUNT_1_BIT,
      .enableDepth = false,
  };

  struct DrawInfo {
    glm::mat4 transform;
    glm::vec4 uvBounds;;
    uint32_t texID;
    uint32_t isMSDF;
  };

  void draw(VkCommandBuffer cmdbuf,
            const std::vector<DrawInfo>& drawInfos,
            uint32_t width,
            uint32_t height);

};

}  // namespace xev
