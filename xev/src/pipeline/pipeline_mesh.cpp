#include <SDL3/SDL_filesystem.h>
#include <xev/logger.h>
#include <xev/pipeline/pipeline_mesh.h>
#include <array>
#include <string>
#include <vector>

namespace xev {

void PipelineMesh::create(VkDevice device,
                          VkFormat color_format,
                          VkFormat depth_format,
                          VkDescriptorSetLayout global_layout,
                          VkSampleCountFlagBits sample_count) {
  VkShaderModule shader_vert, shader_frag;
  const char* base_path = SDL_GetBasePath();
  std::string shader_path = base_path
                                ? std::string(base_path) + "../shaders/mesh.spv"
                                : "shaders/mesh.spv";

  shader_vert = load_shader_module(device, shader_path.c_str());
  shader_frag = load_shader_module(device, shader_path.c_str());

  PipelineInfo pipeinfo{
      .shader_vert = shader_vert,
      .shader_frag = shader_frag,
      .push_const_size = sizeof(PipelineMesh::PushConst),
      .enable_blending = false,
      .topology = VK_PRIMITIV_TOPOLOGY_TRIANGLE_LIST,
      .polygon_mode = VK_POLYGON_MODE_FULL,
      .cull_mode = VK_CULL_MODE_BACK_BIT,
      .front_face = VK_FRONT_FACE_CLOCKWISE,
      .color_format = color_format,
      .depth_format = depth_format,
      .multisample_count = VK_SAMPLE_COUNT_1_BIT,
      .enable_depth = true,
  };
  create(pipeinfo);

  vkDestroyShaderModule(device, shader_vert, nullptr);
  vkDestroyShaderModule(device, shader_frag, nullptr);
}

void PipelineMesh::draw(VkCommandBuffer cmdbuf,
                        const Scene& scene,
                        const Camera& camera,
                        const std::vector<PipelineMesh::Command>& draw_cmds,
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
  for (const auto& cmd : draw_cmds) {
    const Mesh& mesh = scene.meshes[cmd.mesh_id];
    if (cmd.mesh_id != prev_mesh_id) {
      mesh.bind(cmdbuf, vert_addr);
      prev_mesh_id = cmd.mesh_id;
    }

    PushConst push_const = {
        .view_proj = view_proj,
        .model_mat = mesh.get_model_mat(),
        .cam_xyz = camera.pos,
        .scene_addr = scene.scene_device.addr,
        .vert_addr = mesh.get_vert_addr(),
        .mat_id = mesh.get_material_id(),
    };

    vkCmdPushConstants(
        cmdbuf, layout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
        sizeof(PushConst), &push_const);
    vkCmdDrawIndexed(cmdbuf, mesh.get_face_count() * 3, 1, 0, 0, 0);
  }
}

}  // namespace xev
