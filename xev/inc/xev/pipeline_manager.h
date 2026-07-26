#pragma once
#include <xev/volk.h>
#include <string>

namespace xev {

class PipelineMesh;

class PipelineManager {
 public:
  PipelineManager(VkDevice device, VkDescriptorSetLayout descSetLayout)
      : m_device(device), m_descSetLayout(descSet) {}

  PipelineManager(const PipelineManager&) = delete;
  PipelineManager& operator=(const PipelineManager&) = delete;
  PipelineManager(PipelineManager&&) = default;
  PipelineManager& operator=(PipelineManager&&) = default;

  VkShaderModule load_shader(std::string src) const;

  void create(Pipeline& pipe);
  void destroy(Pipeline& pipe) const;

  struct PipelineInfo {
    std::string shaderVertSrc{};
    std::string shaderFragSrc{};
    uint32_t pushConstSize{0};
    bool enableBlending{false};
    bool dynamicDepth{false};
    VkPrimitiveTopology topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
    VkPolygonMode polygonMode{VK_POLYGON_MODE_FULL};
    VkCullModeFlags cullMode{VK_CULL_MODE_BACK_BIT};
    VkFrontFace frontFace{VK_FRONT_FACE_COUNTER_CLOCKWISE};
    VkFormat colorFormat{VK_FORMAT_UNDEFINED};
    VkFormat depthFormat{VK_FORMAT_UNDEFINED};
    VkSampleCountFlagBits multisampleCount{VK_SAMPLE_COUNT_1_BIT};
    bool enableDepth{false};
  };

 private:
  VkDevice m_device{VK_NULL_HANDLE};
  GlobalDescriptorSetLayout m_destSetLayout{VK_NULL_HANDLE};
};

}  // namespace xev
