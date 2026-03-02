#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <xev/backend.h>

namespace xev {

enum SHADER_TYPE {
  S_NULL,
  S_GRAPHICS,
  S_COMPUTE,
};

class Shader {
public:
  Shader();
  ~Shader();
  void load_gfx(VkDevice device, std::string_view filepath) {
    return load(device, filepath, S_GRAPHICS);
  }
  void load(VkDevice device, std::string_view filepath,
            SHADER_TYPE shader_type);
  SHADER_TYPE get_shader_type() { return m_shader_type; }
  const std::vector<VkPipelineShaderStageCreateInfo> &get_stages() const {
    return m_shader_stages;
  }

private:
  VkShaderModule create_shader_mod(VkDevice device);
  void create_shader_stage_gfx(VkShaderModule module);
  void create_shader_stage_com(VkShaderModule module);

#ifdef DEBUG
  std::string m_filepath;
#endif

  std::vector<char> m_shader_src;
  VkShaderModule m_shader_mod = VK_NULL_HANDLE;
  SHADER_TYPE m_shader_type = S_NULL;

  std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
};

} // namespace xev
