#include <xev/renderer3D.h>
#include <xev/resource/scene.h>

namespace xev {

Renderer3D::Renderer3D(std::shared_ptr<Backend> backend,
                       uint32_t width,
                       uint32_t height)
    : m_backend(std::move(backend)) {
  m_pipeline_mesh.create(*m_backend, VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_D32_SFLOAT,
                         VK_SAMPLE_COUNT_1_BIT);

  m_image.load(width, height, *m_backend);
  m_depth_image.load(width, height, *m_backend);
}

Renderer3D::~Renderer3D() {
  m_pipeline_mesh.destroy(*m_backend);
  m_image.unload(*m_backend);
  m_depth_image.unload(*m_backend);
}

const Image& Renderer3D::draw(VkCommandBuffer cmd,
                              const Scene& scene,
                              const Camera& camera,
                              const FrameArg& arg) {
  VkImageMemoryBarrier2 img_barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .image = m_image.image,
      .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
  };
  VkImageMemoryBarrier2 depth_barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                      VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
      .srcAccessMask = 0,
      .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                      VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
      .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                       VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      .image = m_depth_image.image,
      .subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1},
  };
  VkImageMemoryBarrier2 barriers[] = {img_barrier, depth_barrier};
  VkDependencyInfo dep_info = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 2,
      .pImageMemoryBarriers = barriers,
  };
  vkCmdPipelineBarrier2(cmd, &dep_info);

  VkRenderingAttachmentInfo color_attachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = m_image.view,
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {arg.clear_color[0], arg.clear_color[1], arg.clear_color[2],
                     arg.clear_color[3]},
  };

  VkRenderingAttachmentInfo depth_attachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = m_depth_image.view,
      .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .clearValue = {.depthStencil = {1.0f, 0}},

  };

  VkRenderingInfo render_info = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = {{0, 0}, {m_image.width, m_image.height}},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment,
      .pDepthAttachment = &depth_attachment,
  };
  vkCmdBeginRendering(cmd, &render_info);

  draw_mesh(cmd, scene, camera);

  vkCmdEndRendering(cmd);

  img_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  img_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
  img_barrier.dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
  img_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
  img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  dep_info.imageMemoryBarrierCount = 1;
  vkCmdPipelineBarrier2(cmd, &dep_info);

  return m_image;
}

void Renderer3D::draw_mesh(VkCommandBuffer cmd,
                           const Scene& scene,
                           const Camera& camera) {
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
  m_pipeline_mesh.draw(*m_backend, cmd, {m_image.width, m_image.height}, scene,
                       camera, m_mesh_cmds);
}

}  // namespace xev
