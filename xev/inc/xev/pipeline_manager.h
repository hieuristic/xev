#pragma once
#include <xev/volk.h>
#include <string>

namespace xev {

struct RenderPipeline;
struct ComputePipeline;
struct FileSystem;

struct PipelineManager {
  PipelineManager(VkDevice device,
                  VkDescriptorSetLayout descSetLayout,
                  FileSystem& fileSys)
      : m_device(device), m_descSetLayout(descSetLayout), m_fileSys(fileSys) {}

  PipelineManager(const PipelineManager&) = delete;
  PipelineManager& operator=(const PipelineManager&) = delete;
  PipelineManager(PipelineManager&&) = default;
  PipelineManager& operator=(PipelineManager&&) = delete;

  void load_shader(VkShaderModule& mod, std::string path) const;

  void create(RenderPipeline& pipe);
  void destroy(RenderPipeline& pipe) const;

  void create(ComputePipeline& pipe);
  void destroy(ComputePipeline& pipe) const;

 private:
  VkDevice m_device{VK_NULL_HANDLE};
  VkDescriptorSetLayout m_descSetLayout{VK_NULL_HANDLE};
  FileSystem& m_fileSys;
};

}  // namespace xev
