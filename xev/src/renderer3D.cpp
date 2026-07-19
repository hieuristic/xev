#include <xev/global_descriptor_set.h>
#include <xev/logger.h>
#include <xev/pipeline/pipeline_mesh.h>
#include <xev/pipeline_manager.h>
#include <xev/renderer3D.h>
#include <xev/resource/camera.h>
#include <xev/resource/image.h>
#include <xev/resource/scene.h>

namespace xev {

Renderer3D::Renderer3D(PipelineManager& manager,
                       VkDescriptorSetLayout global_layout)
    : m_pipeline_manager(manager) {
  m_pipeline_manager.create(m_pipeline_mesh, VK_FORMAT_R8G8B8A8_UNORM,
                            VK_FORMAT_D32_SFLOAT, global_layout,
                            VK_SAMPLE_COUNT_1_BIT);
}

Renderer3D::~Renderer3D() {
  m_pipeline_manager.destroy(m_pipeline_mesh);
}

void Renderer3D::begin_render(VkCommandBuffer& cmdbuf,
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
  vkCmdBeginRendering(cmdbuf, &render_info);
}

void Renderer3D::draw(VkCommandBuffer cmdbuf,
                      const Image& color_image,
                      const Image& depth_image,
                      const GlobalDescriptorSet& desc_set,
                      const Scene& scene,
                      const Camera& camera,
                      Color4<float> clear_color) {
  XEV_ASSERT(scene.on_device() && color_image.on_device() &&
             depth_image.on_device());

  desc_set.bind(cmdbuf, m_pipeline_mesh.layout);
  prepare_attachment(cmdbuf, color_image, depth_image);

  begin_render(cmdbuf, color_image, depth_image, clear_color);
  draw_mesh(cmdbuf, scene, camera, color_image.width, color_image.height);
  vkCmdEndRendering(cmdbuf);

  prepare_transfer(cmdbuf, color_image);
}

void Renderer3D::draw_mesh(VkCommandBuffer cmdbuf,
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
  m_pipeline_mesh.draw(cmdbuf, scene, camera, m_mesh_cmds, width, height);
}

}  // namespace xev
