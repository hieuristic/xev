#include <xev/camera.h>
#include <xev/color.h>
#include <xev/global_descriptor_set.h>
#include <xev/logger.h>
#include <xev/pipeline/mesh.h>
#include <xev/pipeline_manager.h>
#include <xev/renderer3D.h>
#include <xev/resource/image.h>
#include <xev/resource/scene.h>

namespace xev {

Renderer3D::Renderer3D(PipelineManager& manager, uint32_t numFrames)
    : m_pipelineManager(manager) {
  m_pipeMesh.info.colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
  m_pipeMesh.info.depthFormat = VK_FORMAT_R8G8B8A8_UNORM;
  m_pipeMesh.info.multisampleCount = VK_SAMPLE_COUNT_1_BIT;
  XEV_INFO("At renderer3d, pipemesh vert src: {}",
           m_pipeMesh.info.shaderVertSrc);
  m_pipelineManager.create(m_pipeMesh);

  m_pipelineManager.create(m_pipeSkinning);
  XEV_INFO("Done with pipelinemesh!");
}

Renderer3D::~Renderer3D() {
  m_pipelineManager.destroy(m_pipeMesh);
}

void Renderer3D::prepare_render(VkCommandBuffer& cmdbuf,
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

void Renderer3D::prepare_skinning() {
  // need vertex data available before skinning
  const VkMemoryBarrier2 barrier = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
      .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
      .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
  };
  const VkDependencyInfo info = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .memoryBarrierCount = 1,
      .pMemoryBarriers = &barrier,
  };
  vkCmdPipelineBarrier2(cmd, &info);
}

void Renderer3D::draw(VkCommandBuffer cmdbuf,
                      const Image& colorImage,
                      const Image& depthImage,
                      const GlobalDescriptorSet& desc_set,
                      const Scene& scene,
                      const Camera& camera,
                      Color4<float> clearColor,
                      const Buffer& skeletonBuffer) {
  XEV_ASSERT(scene.on_device() && colorImage.on_device() &&
             depthImage.on_device());

  m_meshInfos.clear();
  m_skinningInfos.clear();

  for (uint32_t i = 0; i < scene.meshes.size(); ++i) {
    const auto& mesh = scene.meshes[i];
    if (!mesh.isVisible) continue;
    if (!camera.can_see(mesh)) continue;

    pipe::Mesh::DrawInfo m_meshInfo{
        .meshId = static_cast<uint32_t>(i),
        .materialId = mesh.get_material_id(),
    };
    m_meshInfos.push_back(m_meshInfo);

    if (!mesh.has_skinned) continue;

    pipe::Skinning::DispatchInfo m_skinningInfo{
        ,
    };
    m_pipeSkinning.dispatch(cmbuf, mesh,
  }

  // question, does the mesh store the index to the "skinned buffer"?

  prepare_skinning(cmdbuf);
  dispatch_skinning();

  desc_set.bind(cmdbuf, m_pipeMesh.layout);
  prepare_attachments(cmdbuf, colorImage, depthImage);
  prepare_render(cmdbuf, colorImage, depthImage, clearColor);
  draw_mesh(cmdbuf, scene, camera, colorImage.width, colorImage.height);

  prepare_transfer(cmdbuf, colorImage);
}

void Renderer3D::dispatch_skinning(VkCommandBuffer cmdbuf) {
  m_pipeSkinning.dispatch(info);
}

void Renderer3D::draw_mesh(VkCommandBuffer cmdbuf,
                           const Scene& scene,
                           const Camera& camera,
                           uint32_t width,
                           uint32_t height) {
  m_pipeMesh.draw(cmdbuf, scene, camera, m_meshInfos, width, height);
  vkCmdEndRendering(cmdbuf);
}

}  // namespace xev
