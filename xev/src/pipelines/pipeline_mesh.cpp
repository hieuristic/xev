#include <xev/pipelines/pipeline_mesh.h>

namespace xev {

PipelineMesh::PipelineMesh(const Backend& backend,
                           VkFormat image_format,
                           VkFormat depth_format,
                           VkSampleCountFlagBits sample_count) {
  VkPipelineShaderStageCreateInfo shader_vert_info =
      backend.create_shader_module("shaders/mesh.vert.slang",
                                   VK_SHADER_STAGE_VERTEX_BIT);
  VkPipelineShaderStageCreateInfo shader_frag_info =
      backend.create_shader_module("shaders/mesh.frag.slang",
                                   VK_SHADER_STAGE_FRAGMENT_BIT);
  VkDescriptorSetLayout desc_set_layout =
      backend.create_bindless_descriptor_set_layout();
  VkPushConstantRange push_const_range {
    .stateFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    .offset = 0, .size = sizeof(m_push_const);
  };
  m_layout = backend.create_pipeline_layout();

  Backend::PipelineInfo pipeline_info = {
      .vert_shader = &shader_vert_info,
      .frag_shader = &shader_frag_info,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .enable_culling = true,
      .multisampling = sample_count,
      .color_format = color_format,
      .depth_format = depth_format,
      .enable_depth_test = true,
      .depth_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
  };

  VkPipeline m_pipeline = backend.create_pipeline(&pipeline_info);
}

PipelineMesh::draw() {}

}  // namespace xev
