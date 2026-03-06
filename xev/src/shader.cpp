#include <fstream>
#include <vector>
#include <xev/logger.h>
#include <xev/shader.h>

namespace xev {

Shader::Shader() {}

Shader::~Shader() {}

void Shader::load(VkDevice device, std::string_view filepath,
                  SHADER_TYPE shader_type) {
  std::ifstream file(std::string{filepath}, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    XEV_ERROR("failed to open {}", filepath);
    return;
  }

  m_shader_src.resize(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(m_shader_src.data(),
            static_cast<std::streamsize>(m_shader_src.size()));
  file.close();

#ifdef DEBUG
  m_filepath = filepath;
#endif

  m_shader_mod = create_shader_mod(device);
  if (m_shader_mod != VK_NULL_HANDLE)
    m_shader_type = shader_type;

  switch (m_shader_type) {
  case S_GRAPHICS:
    create_shader_stage_gfx(m_shader_mod);
    break;
  case S_COMPUTE: // TODO IMPLEMENT THIS
  case S_NULL:
    break;
  }
}

VkShaderModule Shader::create_shader_mod(VkDevice device) {
  VkResult res_;
  VkShaderModule shader_mod = VK_NULL_HANDLE;

  if (m_shader_src.size() == 0) {
    XEV_ERROR("Shader creation failed: source is empty");
    return shader_mod;
  }

  VkShaderModuleCreateInfo mod_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = m_shader_src.size(),
      .pCode = reinterpret_cast<const uint32_t *>(m_shader_src.data()),
  };

  res_ = vkCreateShaderModule(device, &mod_info, nullptr, &shader_mod);

  return shader_mod;
}

void Shader::create_shader_stage_gfx(VkShaderModule module) {
  VkPipelineShaderStageCreateInfo vert_stage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = module,
      .pName = "vert_main",
  };

  VkPipelineShaderStageCreateInfo frag_stage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = module,
      .pName = "frag_main",
  };

  if (m_shader_stages.size() != 0) {
    XEV_INFO("m_shader_stages is not empty before creation");
  }

  m_shader_stages = {vert_stage, frag_stage};
}

} // namespace xev
