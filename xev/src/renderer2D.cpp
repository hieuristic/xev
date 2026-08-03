#include <xev/global_descriptor_set.h>
#include <xev/logger.h>
#include <xev/pipeline/pipeline_raster.h>
#include <xev/pipeline_manager.h>
#include <xev/renderer2D.h>
#include <xev/resource/buffer.h>
#include <xev/resource/image.h>
#include <xev/resource_manager.h>
#include <xev/ui/font.h>

namespace xev {

Renderer2D::Renderer2D(PipelineManager& pipelineManager,
                       ResourceManager& resourceManager,
                       uint32_t numFrameInFlight)
    : m_pipelineManager(pipelineManager), m_resourceManager(resourceManager) {
  m_pipelineRaster.pipeInfo.colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
  m_pipelineRaster.pipeInfo.multisampleCount = VK_SAMPLE_COUNT_1_BIT;
  m_pipelineManager.create(m_pipelineRaster);

  while (numFrameInFlight--) {
    Buffer infoBuf{MAX_DRAW_CALLS * sizeof(PipelineRaster::DrawInfo),
                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                       VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                   VMA_MEMORY_USAGE_AUTO};
    m_resourceManager.alloc(infoBuf);
    m_drawInfoBuffers.push_back(infoBuf);
  }
}

Renderer2D::~Renderer2D() {
  m_pipelineManager.destroy(m_pipelineRaster);
  for (auto& infoBuf : m_drawInfoBuffers) {
    m_resourceManager.free(infoBuf);
  }
}

void Renderer2D::begin_render(VkCommandBuffer cmdbuf,
                              const Image& colorImage,
                              const Color4<float>& clearColor) {
  VkRenderingAttachmentInfo color_attachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = colorImage.view,
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {clearColor.r, clearColor.g, clearColor.b, clearColor.a},
  };
  VkRenderingInfo info = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = {{0, 0}, {colorImage.width, colorImage.height}},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment,
  };
  vkCmdBeginRendering(cmdbuf, &info);
}

void Renderer2D::draw(VkCommandBuffer cmdbuf,
                      const Image& colorImage,
                      const GlobalDescriptorSet& descSet,
                      uint32_t currFrameIdx,
                      const Color4<float>& clearColor) {
  XEV_ASSERT(colorImage.on_device());

  descSet.bind(cmdbuf, m_pipelineRaster.layout);
  prepare_attachments(cmdbuf, colorImage);

  memcpy(m_drawInfoBuffers[0].alloc_info.pMappedData, m_drawInfos.data(),
         m_drawInfos.size() * sizeof(m_drawInfos[0]));
  begin_render(cmdbuf, colorImage, clearColor);
  m_pipelineRaster.draw(cmdbuf, m_drawInfoBuffers[0].addr, m_drawInfos.size(),
                        colorImage.width, colorImage.height);
  vkCmdEndRendering(cmdbuf);

  prepare_transfer(cmdbuf, colorImage);
  m_drawInfos.clear();
}

void Renderer2D::draw_text(const Font& font,
                           std::string text,
                           glm::mat4 transform,
                           float lineWidth) {
  glm::vec2 offset{-1.0f, -1.0f};
  for (const char c : text) {
    if (c == '\n') {
      offset = glm::vec2(0.0f, offset.y + lineWidth);
      continue;
    }

    // TODO: maybe move the transform to the GPU and just
    // add another field called offset ?
    m_drawInfos.emplace_back(PipelineRaster::DrawInfo{
        .transform = transform * font.transform(offset, c),
        .uvBounds = font.atlas_bounds(c),
        .texID = font.tex_id(),
        .isMSDF = 1,
    });
    offset += glm::vec2(font.advance(c), 0);
  }
}

void Renderer2D::draw_image(glm::mat4 transform,
                            glm::vec4 uvBounds,
                            uint32_t texID) {
  m_drawInfos.emplace_back(PipelineRaster::DrawInfo{
      .transform = transform,
      .uvBounds = uvBounds,
      .texID = texID,
      .isMSDF = 1,
  });
}

}  // namespace xev
