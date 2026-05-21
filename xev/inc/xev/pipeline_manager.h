#pragma once

namespace xev {

class PipelineManager {
 public:
  PipelineManager(VkDevice device) : m_device(device) {}
  ~PipelineManager();

  PipelineManager(const PipelineManager&) = delete;
  PipelineManager& operator=(const PipelineManager&) = delete;
  PipelineManager(PipelineManager&&) = default;
  PipelineManager& operator=(PipelineManager&&) = default;

 private:
  VkDevice m_device{VK_NULL_HANDLE};
};

}  // namespace xev
