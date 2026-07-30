#include <xev/ui/font.h>
#include <fstream>
#include <nlohmann/json.hpp>

namespace xev {

Font::Font(const ResourceManager& manager,
           const HotExec& hotExec,
           const std::string& atlasPath,
           const std::string& configPath)
    : m_manager(manager) {
  using json = nlohmann::json;

  // 1. load in atlas data from .bin and .json file
  std::ifstream atlasFile(atlasPath, std::ios::binary);
  if (!atlasFile.is_open()) {
    XEV_ERROR("Can't open font atlas file at %s!", configPath);
  }
  std::ifstream configFile(configPath);
  if (!configFile.is_open()) {
    XEV_ERROR("Can't open font config file at %s!", configPath);
  }
  json configData = json::parse(configFile);
  m_atlas.width = json["atlas"]["width"];
  m_atlas.height = json["atlas"]["height"];
  m_manager.alloc(m_atlas);
  uint64_t atlasArea = m_atlas.width * m_atlas.height;
  std::vector<uint8_t> rgb_data(atlasArea * 3);
  atlasFile.read(reinterpret_cast<char*>(rgb_data.data()), atlasArea * 3);
  m_atlas.host_data.resize(atlasArea * 4);
  for (uint64_t i = 0; i < atlasArea; ++i) {
    m_atlas.host_area[i * 4 + 0] = rgb_data[i * 3 + 0];  // R
    m_atlas.host_area[i * 4 + 1] = rgb_data[i * 3 + 1];  // G
    m_atlas.host_area[i * 4 + 2] = rgb_data[i * 3 + 2];  // B
    m_atlas.host_area[i * 4 + 3] = 255;                  // A (Opaque)
  }

  // 2. upload data to GPU
  Buffer staging{
      atlasArea * 4,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VMA_MEMORY_USAGE_AUTO,
  };
  manager.alloc(staging);
  void* map_ = staging.alloc_info.pMappedData;
  memcpy(map_, img.host_data.data(), size);
  hot_exec.run([&](const VkCommandBuffer cmdbuf) {
    m_atlas.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_atlas.update_layout(cmdbuf, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkBufferImageCopy reg = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .imageOffset = {0, 0, 0},
        .imageExtent =
            {
                static_cast<uint32_t>(m_atlas.width),
                static_cast<uint32_t>(m_atlas.height),
                1,
            },
    };
    vkCmdCopyBufferToImage(cmdbuf, staging.buffer, m_atlas.image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &reg);

    m_atlas.update_layout(cmdbuf, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  });
  manager.free(staging);

  // parse Glyph look-up table
  float w = static_cast<float>(m_atlas.width);
  float h = static_cast<float>(m_atlas.height);
  for (const auto& glyphData : configData["glyphs"]) {
    float left = glyphData["atlasBounds"]["left"];
    float bottom = glyphData["atlasBounds"]["bottom"];
    float right = glyphData["atlasBounds"]["right"];
    float top = glyphData["atlasBounds"]["top"];

    m_glyphs[glyphData["unicode"]] = {
        .advance = glyphData["advance"],
        .planeBounds =
            {
                glyphData["advance"]["planeBounds"]["top"],
                glyphData["advance"]["planeBounds"]["left"],
                glyphData["advance"]["planeBounds"]["right"],
                glyphData["advance"]["planeBounds"]["bottom"],
            },
        .atlasBounds =
            {
                left / w,
                (h - top) / h,
                right / w,
                (h - bottom) / h,
            },
        .advance = glyphData["advance"],
    };
  }

  // parse extra metadata
  m_emSize = configData["metrics"]["emSize"];
  m_lineHeight = configData["metrics"]["lineHeight"];
}

Font::~Font() {
  m_manager.free(m_atlas);
}

void Font::bind(const GlobalDescriptorSet& desc_set) {
  m_texID = desc_set.set(m_atlas);
}

glm::vec4 atlas_bounds(uint32_t c) {
  return m_glyphs[c].atlasBounds;
}

}  // namespace xev
