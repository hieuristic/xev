#pragma once
#include <xev/pipeline/pipeline.h>
#include <xev/volk.h>
#include <glm/glm.hpp>

namespace xev::pipe {

struct Raster : public RenderPipeline {
  struct PushConst {
    VkDeviceAddress infoBuffer;
  };

  struct DrawInfo {
    glm::mat3 transform;
    glm::vec4 uvBounds;
    glm::vec3 color;
    uint32_t texID;
    uint32_t isMSDF;
  };

  Raster() {
    info.shaderVertSrc = "raster.spv";
    info.shaderFragSrc = "raster.spv";
    info.pushConstSize = sizeof(Raster::PushConst);
    info.enableBlending = true;
    info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    info.polygonMode = VK_POLYGON_MODE_FILL;
    info.cullMode = VK_CULL_MODE_NONE;
    info.multisampleCount = VK_SAMPLE_COUNT_1_BIT;
    info.enableDepth = false;
  }

  void draw(VkCommandBuffer cmdbuf,
            VkDeviceAddress infoAddr,
            uint32_t numDraw,
            uint32_t width,
            uint32_t height);
};

}  // namespace xev::pipe
