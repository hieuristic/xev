#include <xev/camera.h>
#include <xev/color.h>
#include <xev/global_descriptor_set.h>
#include <xev/logger.h>
#include <xev/pipeline/pipeline_mesh.h>
#include <xev/pipeline_manager.h>
#include <xev/renderer3D.h>
#include <xev/resource/image.h>
#include <xev/resource/scene.h>

namespace xev {

Renderer3D::Renderer3D(PipelineManager& manager) : m_pipelineManager(manager) {
  m_pipelineMesh.pipeInfo.colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
  m_pipelineMesh.pipeInfo.depthFormat = VK_FORMAT_R8G8B8A8_UNORM;
  m_pipelineMesh.pipeInfo.multisampleCount = VK_SAMPLE_COUNT_1_BIT;
  XEV_INFO("At renderer3d, pipemesh vert src: {}", m_pipelineMesh.pipeInfo.shaderVertSrc);
  m_pipelineManager.create(m_pipelineMesh);
  XEV_INFO("Done with pipelinemesh!");
}

Renderer3D::~Renderer3D() {
  m_pipelineManager.destroy(m_pipelineMesh);
}

void Renderer3D::begin_render(VkCommandBuffer& cmdbuf,
                              const Image& colorImage,
                              const Image& depthImage,
                              const Color4<float> clearColor) {
  VkRenderingAttachmentInfo colorAttachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = colorImage.view,
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {clearColor.r, clearColor.g, clearColor.b, clearColor.a},
  };

  VkRenderingAttachmentInfo depthAttachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = depthImage.view,
      .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .clearValue = {.depthStencil = {1.0f, 0}},
  };

  VkRenderingInfo render_info = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = {{0, 0}, {colorImage.width, colorImage.height}},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachment,
      .pDepthAttachment = &depthAttachment,
  };
  vkCmdBeginRendering(cmdbuf, &render_info);
}

void Renderer3D::draw(VkCommandBuffer cmdbuf,
                      const Image& colorImage,
                      const Image& depthImage,
                      const GlobalDescriptorSet& desc_set,
                      const Scene& scene,
                      const Camera& camera,
                      Color4<float> clearColor) {
  XEV_ASSERT(scene.on_device() && colorImage.on_device() &&
             depthImage.on_device());

  desc_set.bind(cmdbuf, m_pipelineMesh.layout);
  Renderer::prepare_attachments(cmdbuf, colorImage, depthImage);

  begin_render(cmdbuf, colorImage, depthImage, clearColor);
  draw_mesh(cmdbuf, scene, camera, colorImage.width, colorImage.height);
  vkCmdEndRendering(cmdbuf);

  Renderer::prepare_transfer(cmdbuf, colorImage);
}

void Renderer3D::draw_mesh(VkCommandBuffer cmdbuf,
                           const Scene& scene,
                           const Camera& camera,
                           uint32_t width,
                           uint32_t height) {
  m_mesh_cmds.clear();
  for (size_t i = 0; i < scene.meshes.size(); ++i) {
    auto& mesh = scene.meshes[i];
    PipelineMesh::DrawInfo m_cmd{
        .mesh_id = static_cast<uint32_t>(i),
        .material_id = mesh.get_material_id(),
        .to_world = mesh.get_model_mat(),
        .is_skinned = false,
        .skinned_mesh_address = 0,
    };
    m_mesh_cmds.push_back(m_cmd);
  }
  m_pipelineMesh.draw(cmdbuf, scene, camera, m_mesh_cmds, width, height);
}

}  // namespace xev
