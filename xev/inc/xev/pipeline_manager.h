#pragma once
#include <xev/volk.h>
#include <string>

namespace xev {

class Pipeline;
class PipelineMesh;

class PipelineManager {
 public:
  PipelineManager(VkDevice device, VkDescriptorSetLayout descSetLayout)
      : m_device(device), m_descSetLayout(descSetLayout) {}

  PipelineManager(const PipelineManager&) = delete;
  PipelineManager& operator=(const PipelineManager&) = delete;
  PipelineManager(PipelineManager&&) = default;
  PipelineManager& operator=(PipelineManager&&) = default;

  VkShaderModule load_shader(std::string src) const;

  void create(Pipeline& pipe);
  void destroy(Pipeline& pipe) const;

 private:
  VkDevice m_device{VK_NULL_HANDLE};
  VkDescriptorSetLayout m_destSetLayout{VK_NULL_HANDLE};
};

}  // namespace xev
