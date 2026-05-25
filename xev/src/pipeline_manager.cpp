#include <xev/pipeline_manager.h>
#include <xev/pipeline/pipeline_mesh.h>
#include <xev/logger.h>
#include <fstream>
#include <vector>

namespace xev {

void PipelineManager::create(PipelineMesh& pipe,
                             VkFormat format_color,
                             VkFormat format_depth,
                             VkDescriptorSetLayout global_layout,
                             VkSampleCountFlagBits sample_count) const {
  pipe.create(m_device, format_color, format_depth, global_layout, sample_count);
}

void PipelineManager::destroy(PipelineMesh& pipe) const {
  pipe.destroy(m_device);
}

void PipelineManager::create(VkShaderModule& mod, const char* path) const {
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    XEV_ERROR("Failed to read {}", path);
  }

  std::vector<char> m_shader_src;
  auto size = file.tellg();
  m_shader_src.resize(size);
  file.seekg(0, std::ios::beg);
  file.read(m_shader_src.data(), static_cast<std::streamsize>(size));
  file.close();

  VkResult res_;

  VkShaderModuleCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = static_cast<size_t>(size),
      .pCode = reinterpret_cast<const uint32_t*>(m_shader_src.data()),
  };

  res_ = vkCreateShaderModule(m_device, &info, nullptr, &mod);
  XEV_ASSERT_VK(res_, "Failed to load shader module");
}

void PipelineManager::destroy(VkShaderModule& mod) const {
  if (mod != VK_NULL_HANDLE) {
    vkDestroyShaderModule(m_device, mod, nullptr);
    mod = VK_NULL_HANDLE;
  }
}

}  // namespace xev
