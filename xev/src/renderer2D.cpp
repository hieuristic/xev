#include <xev/renderer2D.h>

namespace xev {

Renderer2D::Renderer2D(PipelineManager& manager,
                       VkDescriptorSetLayout global_layout)
    : m_pipeline_manager(manager) {
  m_pipeline_manager.create(m_pipeline_2D, VK_FORMAT_R8G8B8A8_UNORM,
                            VK_FORMAT_D32_SFLOAT, global_layout,
                            VK_SAMPLE_COUNT_1_BIT);
}

Renderer2D::~Renderer2D() {
  m_pipeline_manager.destroy(m_pipeline_2D);
}

void Renderer2D::begin_render(VkCommandBuffer cmdbuf,
                              const Image& color_image,
                              const Color4& clear_color) {
  VkRenderingAttachmentInfo color_attachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = color_image.view,
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {clear_color.r, clear_color.g, clear_color.b,
                     clear_color.a},
  };
  VkRenderingInfo render_info = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = {{0, 0}, {color_image.width, color_image.height}},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment,
  };
  vkCmdBeginRendering(cmdbuf, &render_info);
}

void Renderer2D::draw(VkCommandBuffer cmdbuf,
                      const Image& color_image,
                      const GlobalDescriptorSet& desc_set,
                      const std::vector<std::pair<Raster, uint32_t>>& rasters,
                      Color4<float> clear_color) {
  XEV_ASSERT(color_image.on_device() && depth_image.on_device());

  desc_set.bind(cmdbuf, m_pipeline_raster.layout);
  prepare_attachment(cmdbuf, color_image);

  begin_render(cmdbuf, color_image, clear_color);
  m_pipeline_raster.draw(cmdbuf, rasters, color_image.width,
                         color_image.height);
  vkCmdEndRendering(cmdbuf);

  prepare_transfer(cmdbuf, color_image);
}

}  // namespace xev
