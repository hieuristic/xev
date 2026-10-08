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

Renderer3D::Renderer3D(PipelineManager& pipelineManager,
                       ResourceManager& resourceManager,
                       uint32_t numFrameInFlight)
    : m_pipelineManager(pipelineManager), m_resourceManager(resourceManager) {
  m_pipeMesh.info.colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
  m_pipeMesh.info.depthFormat = VK_FORMAT_R8G8B8A8_UNORM;
  m_pipeMesh.info.multisampleCount = VK_SAMPLE_COUNT_1_BIT;
  m_pipelineManager.create(m_pipeMesh);
  m_resourceManager.create(m_pipeSkinning);

  m_resourceManager.alloc(m_bufArrBoneTransforms);
}

Renderer3D::~Renderer3D() {
  m_pipelineManager.destroy(m_pipeMesh);
  m_pipelineManager.destroy(m_pipeSkinning);
  m_resourceManager.free(m_bufArrBoneTransform);
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

void Renderer3D::wait_skinning() {
  // need vertex data available before skinning
  const VkMemoryBarrier2 barrier = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
      .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
      .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
  };
  const VkDependencyInfo info = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .memoryBarrierCount = 1,
      .pMemoryBarriers = &barrier,
  };
  vkCmdPipelineBarrier2(cmdbuf, &info);
}

void Renderer3D::draw(const VkCommandBuffer cmdbuf,
                      const Image& colorImage,
                      const Image& depthImage,
                      const GlobalDescriptorSet& desc_set,
                      const Scene& scene,
                      const Camera& camera,
                      const Color4<float>& clearColor,
                      const uint32_t currFrameIdx = 0) {
  XEV_ASSERT(scene.on_device() && colorImage.on_device() &&
             depthImage.on_device());

  m_meshInfos.clear();
  m_skinningInfos.clear();

  bool isAnimated = m_numBones[currFrameIdx] == 0;

  for (uint32_t i = 0; i < scene.meshes.size(); ++i) {
    const auto& mesh = scene.meshes[i];
    if (!mesh.isVisible) continue;
    if (!camera.can_see(mesh)) continue;

    m_meshInfos.push_back({
        .meshId = static_cast<uint32_t>(i),
        .materialId = mesh.get_material_id(),
    });

    if (isAnimated || !mesh.is_skinned) continue;

    m_skinningInfos.push_back({
        .boneTransform = m_bufArrBoneTransform.get_addr(currFrameIdx),
        .palette = mesh.get_palette_addr(),
        .iBuf = mesh.get_vert_addr(),
        .oBuf = mesh.get_skinned_vert_addr(currFrameIdx),
        .offset = 0,
        .count = mesh.get_vertex_count(),
    });
    m_pipeSkinning.dispatch(cmbuf, mesh,
  }

  if (!m_skinningInfos) {
    dispatch_skinning(cmdbuf);
    wait_skinning(cmdbuf);
  }

  desc_set.bind(cmdbuf, m_pipeMesh.layout);
  prepare_attachments(cmdbuf, colorImage, depthImage);
  prepare_render(cmdbuf, colorImage, depthImage, clearColor);
  draw_mesh(cmdbuf, scene, camera, colorImage.width, colorImage.height,
            currFrameIdx, isAnimated);

  prepare_transfer(cmdbuf, colorImage);
}

void Renderer3D::upload_skinning(uint32_t currFrameIdx,
                                 std::span<const glm::mat4> data) {
  // URGENT TODO This function is incomplete without proper syncing
  // this it update regularly, direct upload can cause performance issue
  XEV_ASSERT(currFrameIdx < m_skinningBuffers.size());
  XEV_ASSERT(data.size() <= MAX_BONES);
  m_skinningBuffers[currFrameIdx].write(data.data(), data.size_bytes());
  m_numBones[currFrameIdx] = static_cast<uint32_t>(data.size());
}

void Renderer3D::dispatch_skinning(VkCommandBuffer cmdbuf) {
  m_pipeSkinning.dispatch(cmdbuf, m_skinningInfos);
}

void Renderer3D::draw_mesh(VkCommandBuffer cmdbuf,
                           const Scene& scene,
                           const Camera& camera,
                           const uint32_t width,
                           const uint32_t height,
                           const bool isAnimated) {
  m_pipeMesh.draw(cmdbuf, scene, camera, m_meshInfos, width, height);
  vkCmdEndRendering(cmdbuf);
}

}  // namespace xev
