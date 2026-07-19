#pragma once
#include <xev/resource/resource.h>
#include <xev/volk.h>

namespace xev {

class Pipeline {
 public:
  virtual void draw() {}
  VkPipelineLayout layout;
  VkPipeline pipeline;
  static VkShaderModule load_shader_module() const;
  void create(VkDevice device, const PipelineInfo& pipeinfo);
  void destroy(VkDevice device);

  struct PipelinePushConstRange {
    uint32_t range_cnt;
    const VkPushConstantRange&
  }

  struct PipelineInfo {
    VkShaderModule shader_vert{};
    VkShaderModule shader_frag{};
    uint32_t push_const_size{0};
    bool enable_blending{false};
    bool dynamic_depth{false};
    VkPrimitiveTopology topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
    VkPolygonMode polygon_mode{VK_POLYGON_MODE_FULL};
    VkCullModeFlags cull_mode{VK_CULL_MODE_BACK_BIT};
    VkFrontFace front_face{VK_FRONT_FACE_COUNTER_CLOCKWISE};
    VkFormat color_format{VK_FORMAT_UNDEFINED};
    VkFormat depth_format{VK_FORMAT_UNDEFINED};
    VkSampleCountFlagBits multisample_count{VK_SAMPLE_COUNT_1_BIT};
    bool enable_depth{false};
  };
};

}  // namespace xev
