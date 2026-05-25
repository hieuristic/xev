#pragma once
#include <xev/volk.h>

namespace xev {

class PipelineMesh;

class PipelineManager {
 public:
  PipelineManager(VkDevice device) : m_device(device) {}

  PipelineManager(const PipelineManager&) = delete;
  PipelineManager& operator=(const PipelineManager&) = delete;
  PipelineManager(PipelineManager&&) = default;
  PipelineManager& operator=(PipelineManager&&) = default;

  void create(PipelineMesh& pipe,
              VkFormat format_color,
              VkFormat format_depth,
              VkDescriptorSetLayout global_layout,
              VkSampleCountFlagBits sample_count) const;
  void create(VkShaderModule& mod, const char* path) const;

  void destroy(PipelineMesh& pipe) const;
  void destroy(VkShaderModule& mod) const;

 private:
  VkDevice m_device{VK_NULL_HANDLE};
};

}  // namespace xev
