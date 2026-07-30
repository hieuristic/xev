#pragma once
#include <xev/resource/resource.h>
#include <xev/resource_manager.h>
#include <xev/volk.h>

namespace xev {

struct PipelineInfo {
  std::string shaderVertSrc{};
  std::string shaderFragSrc{};
  uint32_t pushConstSize{0};
  bool enableBlending{false};
  bool dynamicDepth{false};
  VkPrimitiveTopology topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
  VkPolygonMode polygonMode{VK_POLYGON_MODE_FILL};
  VkCullModeFlags cullMode{VK_CULL_MODE_BACK_BIT};
  VkFrontFace frontFace{VK_FRONT_FACE_COUNTER_CLOCKWISE};
  VkFormat colorFormat{VK_FORMAT_UNDEFINED};
  VkFormat depthFormat{VK_FORMAT_UNDEFINED};
  VkSampleCountFlagBits multisampleCount{VK_SAMPLE_COUNT_1_BIT};
  bool enableDepth{false};
};

class Pipeline {
 public:
  virtual void draw() {}
  virtual void get_pipeline_info(PipelineInfo& pipeInfo) const;
  VkPipelineLayout layout;
  VkPipeline pipeline;
};

}  // namespace xev
