#include <SDL3/SDL_filesystem.h>
#include <xev/pipeline/pipeline_mesh.h>
#include <string>

namespace xev {

void PipelineMesh::create(const Backend& backend,
                          VkFormat color_format,
                          VkFormat depth_format,
                          VkSampleCountFlagBits sample_count) {
  VkShaderModule shader_vert, shader_frag;
  const char* base_path = SDL_GetBasePath();
  std::string shader_path = base_path
                                ? std::string(base_path) + "../shaders/mesh.spv"
                                : "shaders/mesh.spv";

  shader_vert = backend.create_shader_module(shader_path.c_str());
  shader_frag = backend.create_shader_module(shader_path.c_str());

  VkDescriptorSetLayout desc_set_layout =
      backend.create_bindless_descriptor_set_layout();

  VkPushConstantRange push_const_range = {
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      .offset = 0,
      .size = sizeof(PipelineMesh::PushConst),
  };

  std::vector<VkDescriptorSetLayout> layouts = {desc_set_layout};
  std::vector<VkPushConstantRange> ranges = {push_const_range};
  m_layout = backend.create_pipeline_layout(layouts, ranges);

  Backend::PipelineInfo pipeline_info = {
      .vert_shader = shader_vert,
      .frag_shader = shader_frag,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .polygon = VK_POLYGON_MODE_FILL,
      .samples = sample_count,
      .color_format = color_format,
      .depth_format = depth_format,
      .enable_culling = false,
      .enable_depth_test = false,
      .depth_op = VK_COMPARE_OP_LESS_OR_EQUAL,

      .enable_blend = false,
      .blend_op = VK_BLEND_OP_ADD,
      .blend_src_factor = VK_BLEND_FACTOR_ONE,
      .blend_dst_alpha_factor = VK_BLEND_FACTOR_ZERO,
      .layout = m_layout,
      .vertex_bindings =
          {
              {
                  .binding = 0,
                  .stride = sizeof(Mesh::Vertex),
                  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
              },
          },
      .vertex_attributes = {
          {.location = 0,
           .binding = 0,
           .format = VK_FORMAT_R32G32B32_SFLOAT,
           .offset = offsetof(Mesh::Vertex, position)},  // 0
          {.location = 1,
           .binding = 0,
           .format = VK_FORMAT_R32G32B32_SFLOAT,
           .offset = offsetof(Mesh::Vertex, normal)},  // 12
          {.location = 2,
           .binding = 0,
           .format = VK_FORMAT_R32G32_SFLOAT,
           .offset = offsetof(Mesh::Vertex, uv)},  // 24,
      }};

  m_pipeline = backend.create_pipeline(pipeline_info);

  backend.destroy_shader_module(shader_vert);
  backend.destroy_shader_module(shader_frag);
}

void PipelineMesh::destroy(const Backend& backend) {
  backend.destroy_pipeline_layout(m_layout);
  backend.destroy_pipeline(m_pipeline);
}

void PipelineMesh::draw(const Backend& backend,
                        VkCommandBuffer cmdbuf,
                        VkExtent2D ext,
                        const Scene& scene,
                        const Camera& camera,
                        const std::vector<PipelineMesh::Command>& draw_cmds) {
  XEV_INFO("DRAWING Camera x:{}, y:{}, z:{}", camera.pos.x, camera.pos.y,
           camera.pos.z);
  vkCmdBindPipeline(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

  VkViewport viewport = {
      .x = 0,
      .y = 0,
      .width = (float)ext.width,
      .height = (float)ext.height,
      .minDepth = 0.f,
      .maxDepth = 1.f,
  };
  vkCmdSetViewport(cmdbuf, 0, 1, &viewport);

  VkRect2D scissor = {
      .offset = {},
      .extent = ext,
  };
  vkCmdSetScissor(cmdbuf, 0, 1, &scissor);

  // In real renderer we'd bind global descriptor sets here
  // vkCmdBindDescriptorSets(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layout,
  // 0, 1, &backend.get_desc_set(), 0, nullptr);

  glm::mat4 vp = camera.create_vp_mat();

  uint32_t prev_mesh_id = static_cast<uint32_t>(-1);

  for (const auto& cmd : draw_cmds) {
    const Mesh& mesh = scene.meshes[cmd.mesh_id];
    if (cmd.mesh_id != prev_mesh_id) {
      prev_mesh_id = cmd.mesh_id;
      vkCmdBindIndexBuffer(cmdbuf, mesh.get_face_buffer(), 0,
                           VK_INDEX_TYPE_UINT32);
      VkBuffer tmp_buffer = mesh.get_vert_buffer();
      VkDeviceSize tmp_size = 0;
      vkCmdBindVertexBuffers(cmdbuf, 0, 1, &tmp_buffer, &tmp_size);
    }

    PushConst push_const = {
        .mvp = vp * cmd.to_world,
        .scene_uniform = 0,  // Not implemented yet
        .vertex_buffer = 0,  // BufferDeviceAddress not set up yet
        .material_id = cmd.material_id,
        .padding = 0,
    };

    vkCmdPushConstants(
        cmdbuf, m_layout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
        sizeof(PushConst), &push_const);

    // Render all primitives within the mesh
    for (const auto& pri : mesh.get_primitives()) {
      vkCmdDrawIndexed(cmdbuf, pri.flength * 3, 1, pri.foffset * 3, pri.voffset,
                       0);
    }
  }
}

}  // namespace xev
