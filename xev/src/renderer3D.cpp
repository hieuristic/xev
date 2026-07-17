#include <xev/logger.h>
#include <xev/pipeline_manager.h>
#include <xev/renderer3D.h>
#include <xev/resource/scene.h>

namespace xev {

Renderer3D::Renderer3D(PipelineManager& pipeline_manager,
                       VkDescriptorSetLayout global_layout)
    : m_pipeline_manager(pipeline_manager) {
  m_pipeline_manager.create(m_pipeline_mesh, VK_FORMAT_R8G8B8A8_UNORM,
                            VK_FORMAT_D32_SFLOAT, global_layout,
                            VK_SAMPLE_COUNT_1_BIT);
}

Renderer3D::~Renderer3D() {
  m_pipeline_manager.destroy(m_pipeline_mesh);
}

void Renderer3D::prepare_attachments(VkCommandBuffer& cmd,
                                     const Image& color_image,
                                     const Image& depth_image) {
  std::array<VkImageMemoryBarrier2, 2> barriers = {
      {{
           // Color barrier
           .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
           .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
           .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
           .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
           .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
           .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
           .image = color_image.image,
           .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
       },
       {
           // Depth barrier
           .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
           .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                           VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
           .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                           VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
           .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
           .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
           .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
           .image = depth_image.image,
           .subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1},
       }}};

  VkDependencyInfo info = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = (uint32_t)barriers.size(),
      .pImageMemoryBarriers = barriers.data(),
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
                              const Image& color_image,
                              const Image& depth_image,
                              const Color4<float> clear_color) {
  VkRenderingAttachmentInfo color_attachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = color_image.view,
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {clear_color.r, clear_color.g, clear_color.b,
                     clear_color.a},
  };

  VkRenderingAttachmentInfo depth_attachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = depth_image.view,
      .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .clearValue = {.depthStencil = {1.0f, 0}},
  };

  VkRenderingInfo render_info = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = {{0, 0}, {color_image.width, color_image.height}},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment,
      .pDepthAttachment = &depth_attachment,
  };
  vkCmdBeginRendering(cmd, &render_info);
}

void Renderer3D::end_render(VkCommandBuffer& cmd) {
  vkCmdEndRendering(cmd);
}

void Renderer3D::draw(VkCommandBuffer cmd,
                      const Image& color_image,
                      const Image& depth_image,
                      const Scene& scene,
                      const Camera& camera,
                      Color4<float> clear_color) {
  XEV_ASSERT(scene.on_device() && color_image.on_device() &&
             depth_image.on_device());

  prepare_attachments(cmd, color_image, depth_image);

  begin_render(cmd, color_image, depth_image, clear_color);
  draw_mesh(cmd, scene, camera, color_image.width, color_image.height);
  end_render(cmd);

  prepare_transfer(cmd, color_image);
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
        .material_id = mesh.get_material_id(),
        .to_world = mesh.get_model_mat(),
        .is_skinned = false,
        .skinned_mesh_address = 0,
    };
    m_mesh_cmds.push_back(m_cmd);
  }
  m_pipeline_mesh.draw(cmd, scene, camera, m_mesh_cmds, width, height);
}

}  // namespace xev
