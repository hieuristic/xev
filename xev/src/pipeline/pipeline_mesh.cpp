#include <xev/logger.h>
#include <xev/pipeline/pipeline_mesh.h>

namespace xev {

void PipelineMesh::draw(VkCommandBuffer cmdbuf,
                        const Scene& scene,
                        const Camera& camera,
                        const std::vector<PipelineMesh::DrawInfo>& drawInfos,
                        uint32_t width,
                        uint32_t height) {
  vkCmdBindPipeline(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  VkViewport viewport = {
      .x = 0,
      .y = 0,
      .width = static_cast<float>(width),
      .height = static_cast<float>(height),
      .minDepth = 0.f,
      .maxDepth = 1.f,
  };
  vkCmdSetViewport(cmdbuf, 0, 1, &viewport);

  VkRect2D scissor = {
      .offset = {},
      .extent = {width, height},
  };
  vkCmdSetScissor(cmdbuf, 0, 1, &scissor);

  glm::mat4 view_proj = camera.create_vp_mat();

  uint32_t prev_mesh_id = static_cast<uint32_t>(-1);

  VkDeviceAddress vert_addr{0};
  for (const auto& info : drawInfos) {
    const Mesh& mesh = scene.meshes[info.mesh_id];
    if (info.mesh_id != prev_mesh_id) {
      mesh.bind(cmdbuf, vert_addr);
      prev_mesh_id = info.mesh_id;
    }

    PushConst push_const = {
        .viewProj = view_proj,
        .modelMat = mesh.get_model_mat(),
        .camXYZ = camera.pos,
        .sceneBuffer = scene.scene_device.addr,
        .vertexBuffer = mesh.get_vert_addr(),
        .matID = mesh.get_material_id(),
    };

    vkCmdPushConstants(
        cmdbuf, layout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
        sizeof(PushConst), &push_const);
    vkCmdDrawIndexed(cmdbuf, mesh.get_face_count() * 3, 1, 0, 0, 0);
  }
}

}  // namespace xev
