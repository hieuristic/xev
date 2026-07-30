#pragma once
#include <unordered_map>
#include <vector>

namespace xev {

class Font {
 public:
  struct Glyph {
    float advance{0.0f};
    glm::vec4 planeBounds{0, 0, 0, 0};
    glm::vec4 atlasBounds{0, 0, 0, 0};
  };

  Font(const ResourceManager& manager,
       const HotExec& hotExec,
       const std::string& atlas_path,
       const std::string& config_path);
  ~Font();
  void bind(const GlobalDescriptorSet& desc_set);
  glm::vec4 plane_bounds(uint32_t c); // return left top right bottom
  glm::vec4 atlas_bounds(uint32_t c); // return left top right bottom
  glm::mat4 transform(const glm::vec2& offset);

 private:
  float m_emSize = 1;
  float m_lineHeight = 1.25;
  const ResourceManager& m_manager;
  uint32_t m_texID;
  Image m_atlas{
      VK_FORMAT_R8G8B8A8_UNORM,
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
  };
  std::unordered_map<uint32_t, Glyph> m_glyphs;
};

}  // namespace xev
