#pragma once
#include <vector>

namespace xev {

class Font {
 public:
  Font(const ResourceManager& manager, void char* path);
  ~Font();
  void bind(const GlobalDescriptorSet& desc_set);
  float4 get_uvs(

 private:
  const ResourceManager& m_manager;
  Image fontAtlas{
      VK_FORMAT_R8G8B8A8_UNORM,
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
  };
  uint32_t m_texid;
};

}  // namespace xev
