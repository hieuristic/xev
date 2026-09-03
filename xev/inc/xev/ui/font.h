#pragma once
#include <xev/resource/image.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <string_view>

namespace xev {

struct HotExec;
struct FileSystem;
struct ResourceManager;
struct GlobalDescriptorSet;

struct Font {
  struct Glyph {
    float advance{0.0f};
    glm::vec4 planeBounds{0, 0, 0, 0};
    glm::vec4 atlasBounds{0, 0, 0, 0};
  };

  Font(const ResourceManager& manager,
       const HotExec& hotExec,
       const FileSystem& fileSys,
       const std::string& atlas_path,
       const std::string& config_path);
  ~Font();
  void bind(GlobalDescriptorSet& desc_set);
  glm::vec4 plane_bounds(uint32_t c) const;  // return left top right bottom
  glm::vec4 atlas_bounds(uint32_t c) const;  // return left top right bottom
  glm::mat3 transform(const glm::vec2& offset, uint32_t c) const;
  float advance(uint32_t c) const;
  glm::vec2 measure(std::string_view text) const;
  uint32_t tex_id() const;

  float emSize = 1;
  float lineHeight = 1.25;

 private:
  const ResourceManager& m_manager;
  uint32_t m_texID;
  Image m_atlas{
      VK_FORMAT_R8G8B8A8_UNORM,
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
  };
  std::unordered_map<uint32_t, Glyph> m_glyphs;
};

}  // namespace xev
