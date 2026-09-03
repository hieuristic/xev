#include <xev/filesystem/fs.h>
#include <xev/global_descriptor_set.h>
#include <xev/hot_exec.h>
#include <xev/logger.h>
#include <xev/resource/buffer.h>
#include <xev/resource/image.h>
#include <xev/resource_manager.h>
#include <xev/ui/font.h>

#include <nlohmann/json.hpp>

namespace xev {

Font::Font(const ResourceManager& manager,
           const HotExec& hotExec,
           const FileSystem& fileSys,
           const std::string& atlasPath,
           const std::string& configPath)
    : m_manager(manager) {
  std::vector<uint8_t> configBuffer = fileSys.read(configPath);
  std::vector<uint8_t> atlasBuffer = fileSys.read(atlasPath);

  nlohmann::json configData =
      nlohmann::json::parse(configBuffer.begin(), configBuffer.end());
  m_atlas.width = configData["atlas"]["width"];
  m_atlas.height = configData["atlas"]["height"];
  m_manager.alloc(m_atlas);
  uint64_t atlasArea = m_atlas.width * m_atlas.height;

  m_atlas.host_data.resize(atlasArea * 4);
  for (uint64_t i = 0; i < atlasArea; ++i) {
    m_atlas.host_data[i * 4 + 0] = atlasBuffer[i * 3 + 0];  // R
    m_atlas.host_data[i * 4 + 1] = atlasBuffer[i * 3 + 1];  // G
    m_atlas.host_data[i * 4 + 2] = atlasBuffer[i * 3 + 2];  // B
    m_atlas.host_data[i * 4 + 3] = 255;                     // A (Opaque)
  }

  // 2. upload data to GPU
  Buffer staging{
      atlasArea * 4,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VMA_MEMORY_USAGE_AUTO,
  };
  manager.alloc(staging);
  void* map_ = staging.alloc_info.pMappedData;
  memcpy(map_, m_atlas.host_data.data(), m_atlas.host_data.size());
  hotExec.run([&](const VkCommandBuffer cmdbuf) {
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
    Glyph g{};
    g.advance = glyphData["advance"];

    if (glyphData.contains("planeBounds")) {
      g.planeBounds = {
          glyphData["planeBounds"]["left"],
          glyphData["planeBounds"]["top"],
          glyphData["planeBounds"]["right"],
          glyphData["planeBounds"]["bottom"],
      };
    }

    if (glyphData.contains("atlasBounds")) {
      float left = glyphData["atlasBounds"]["left"];
      float bottom = glyphData["atlasBounds"]["bottom"];
      float right = glyphData["atlasBounds"]["right"];
      float top = glyphData["atlasBounds"]["top"];
      g.atlasBounds = {
          left / w,
          bottom / h,
          right / w,
          top / h,
      };
    }
    m_glyphs[glyphData["unicode"]] = g;
  };

  // parse extra metadata
  emSize = configData["metrics"]["emSize"];
  lineHeight = configData["metrics"]["lineHeight"];
}

Font::~Font() {
  m_manager.free(m_atlas);
}

glm::mat3 Font::transform(const glm::vec2& offset, uint32_t c) const {
  glm::vec4 bounds = plane_bounds(c);
  float w = (bounds.z - bounds.x) * 0.5f;
  float h = (bounds.w - bounds.y) * 0.5f;
  // glm::vec2 origin = offset + glm::vec2(bounds.x + w, bounds.w + h);
  glm::vec2 origin = offset + glm::vec2(bounds.x + w, 1.0f - (bounds.y + h));

  return glm::mat3(glm::vec3(w, 0.0f, 0.0f), glm::vec3(0.0f, h, 0.0f),
                   glm::vec3(origin.x, origin.y, 1.0f));
}

void Font::bind(GlobalDescriptorSet& descSet) {
  m_texID = descSet.set(m_atlas);
}

float Font::advance(uint32_t c) const {
  auto it = m_glyphs.find(c);
  return (it != m_glyphs.end()) ? it->second.advance : 0.0f;
}

uint32_t Font::tex_id() const {
  return m_texID;
}

glm::vec4 Font::plane_bounds(uint32_t c) const {
  auto it = m_glyphs.find(c);
  return (it != m_glyphs.end()) ? it->second.planeBounds : glm::vec4(0.0f);
}

glm::vec4 Font::atlas_bounds(uint32_t c) const {
  auto it = m_glyphs.find(c);
  return (it != m_glyphs.end()) ? it->second.atlasBounds : glm::vec4(0.0f);
}

glm::vec2 Font::measure(std::string_view text) const {
  float maxLineWidth = 0.0f;
  float currentLineWidth = 0.0f;
  uint32_t lineCount = 1;

  for (char c : text) {
    if (c == '\n') {
      maxLineWidth = std::max(maxLineWidth, currentLineWidth);
      currentLineWidth = 0.0f;
      lineCount++;
      continue;
    }
    currentLineWidth += advance(c);
  }
  maxLineWidth = std::max(maxLineWidth, currentLineWidth);
  return glm::vec2(maxLineWidth, lineCount * lineHeight);
}

}  // namespace xev
