#include <xev/logger.h>
#include <xev/pipeline_manager.h>
#include <xev/renderer3D.h>
#include <xev/resource/scene.h>

namespace xev {

Renderer3D::Renderer3D(PipelineManager& pipeline_manager,
                       VkDescriptorSetLayout global_layout)
    : m_pipeline_manager(pipeline_manager) {
  m_pipeline_manager.create(m_pipeline_mesh, VK_FORMAT_B8G8R8A8_SRGB,
                            VK_FORMAT_D32_SFLOAT, global_layout,
                            VK_SAMPLE_COUNT_1_BIT);
}

Renderer3D::~Renderer3D() {
  m_pipeline_manager.destroy(m_pipeline_mesh);
}

void Renderer3D::prepare_image(VkCommandBuffer& cmd, const Image& image) {
  VkImageMemoryBarrier2 barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .image = image.image,
      .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
  };
  VkDependencyInfo info = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier,
  };
  vkCmdPipelineBarrier2(cmd, &info);
}

void Renderer3D::prepare_transfer(VkCommandBuffer& cmd, const Image& image) {
  VkImageMemoryBarrier2 barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
      .dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      .image = image.image,
      .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
  };
  VkDependencyInfo info = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier,
  };
  vkCmdPipelineBarrier2(cmd, &info);
}

void Renderer3D::begin_render(VkCommandBuffer& cmd,
                              const Image& image,
                              const Color4<float> clear_color) {
  VkRenderingAttachmentInfo color_attachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = image.view,
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {clear_color.r, clear_color.g, clear_color.b,
                     clear_color.a},
  };

  VkRenderingInfo render_info = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = {{0, 0}, {image.width, image.height}},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment,
  };
  vkCmdBeginRendering(cmd, &render_info);
}

void Renderer3D::end_render(VkCommandBuffer& cmd) {
  vkCmdEndRendering(cmd);
}

void Renderer3D::draw(VkCommandBuffer cmd,
                      const Image& image,
                      const Scene& scene,
                      const Camera& camera,
                      Color4<float> clear_color) {
  XEV_ASSERT(scene.on_device() && image.on_device());

  prepare_image(cmd, image);

  begin_render(cmd, image, clear_color);
  draw_mesh(cmd, scene, camera, image.width, image.height);
  end_render(cmd);

  prepare_transfer(cmd, image);
}

void Renderer3D::draw_mesh(VkCommandBuffer cmd,
                           const Scene& scene,
                           const Camera& camera,
                           uint32_t width,
                           uint32_t height) {
  m_mesh_cmds.clear();
  for (size_t i = 0; i < scene.meshes.size(); ++i) {
    auto& mesh = scene.meshes[i];
    PipelineMesh::Command m_cmd{
        .mesh_id = static_cast<uint32_t>(i),
        .material_id = 0,  // Unused for now
        .to_world = mesh.get_model_mat(),
        .is_skinned = false,
        .skinned_mesh_address = 0,
    };
    m_mesh_cmds.push_back(m_cmd);
  }
  m_pipeline_mesh.draw(cmd, scene, camera, m_mesh_cmds, width, height);
}

}  // namespace xev
