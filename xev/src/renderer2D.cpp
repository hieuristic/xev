#include <xev/renderer2D.h>

namespace xev {

Rederer2D::Renderer2D(PipelineManager& pipelineManager,
                      ResourceManager& resourceManager,
                      VkDescriptorSetLayout descSetLayout,
                      uint32_t numFrameInFlight)
    : m_pipelineManager(pipelineManager),
      m_resourceManager(resourceManager),
      m_descSetLayout(descSetLayout) {
  m_pipelineManager.create(m_pipelineRaster, VK_FORMAT_R8G8B8A8_UNORM,
                           VK_FORMAT_D32_SFLOAT, m_descSetLayout,
                           VK_SAMPLE_COUNT_1_BIT);

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
  m_pipelineManager.destroy(m_pipeline_2D);
  for (auto& infoBuf : m_drawInfoBuffers) {
    m_resourceManager.free(infoBuf);
  }
}

void Renderer2D::begin_render(VkCommandBuffer cmdbuf,
                              const Image& colorImage,
                              const Color4& clearColor) {
  VkRenderingAttachmentInfo color_attachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = colorImage.view,
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
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
                      Color4<float> clearColor) {
  XEV_ASSERT(colorImage.on_device() && depth_image.on_device());

  descSet.bind(cmdbuf, m_pipelineRaster.layout);
  prepare_attachment(cmdbuf, colorImage);

  memcpy(m_drawInfoBuffers[0].alloc_info.pMappedData, m_drawInfos.data(),
         m_drawInfos.size() * sizeof(m_drawInfos[0]));
  begin_render(cmdbuf, colorImage, clearColor);
  m_pipelineRaster.draw(cmdbuf, m_drawInfoBuffers[0].addr, m_drawInfos.size(),
                        colorImage.width, colorImage.height);
  vkCmdEndRendering(cmdbuf);

  prepare_transfer(cmdbuf, colorImage);
  m_drawInfos.clear();
}

void Renderer2D::draw_text(Font font,
                           std::string text,
                           glm::mat4 transform,
                           uint32_t lineWidth) {
  glm::vec2 offset = 0;
  for (const char c : text) {
    if (c == '\n') {
      offset += glm::vec2(0, lineWidth);
      continue;
    }

    // TODO: maybe move the transform to the GPU and just
    // add another field called offset ?
    glm::mat4 charTran = transform * font.transform(offset);
    glm::vec2 charUVTL = font.top_left(c);
    glm::vec2 charUVBR = font.bot_right(c);
    offset += glm::vec2(font.width(c), 0);

    m_drawInfos.emplace_back({
        .transform = charTran,
        .uvTopLeft = charUVTL,
        .uvBotRight = charUVBR,
        .texID = font.texID,
        .isMSDF = 1,
    });
  }
}

void Renderer2D::draw_image(glm::mat4 transform,
                            glm::vec2 uvTopLeft,
                            glm::vec2 uvBotRight,
                            uint32_t texID) {
  m_drawInfos.emplace_back({
      .transform = transform,
      .uvTopLeft = uvTopLeft,
      .uvBotRight = uvBotRight,
      .texID = texID,
      .isMSDF = 1,
  });
}

}  // namespace xev
