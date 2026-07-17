#include <SDL3/SDL_filesystem.h>
#include <xev/logger.h>
#include <xev/pipeline/pipeline_mesh.h>
#include <array>
#include <fstream>
#include <string>
#include <vector>

namespace xev {

static VkShaderModule load_shader_module(VkDevice device, const char* path) {
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    XEV_ERROR("Failed to read {}", path);
  }

  std::vector<char> m_shader_src;
  auto size = file.tellg();
  m_shader_src.resize(size);
  file.seekg(0, std::ios::beg);
  file.read(m_shader_src.data(), static_cast<std::streamsize>(size));
  file.close();

  VkResult res_;
  VkShaderModule mod;

  VkShaderModuleCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = static_cast<size_t>(size),
      .pCode = reinterpret_cast<const uint32_t*>(m_shader_src.data()),
  };

  res_ = vkCreateShaderModule(device, &info, nullptr, &mod);
  XEV_ASSERT_VK(res_, "Failed to load shader module");
  return mod;
}

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

  VkPushConstantRange push_const_range = {
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      .offset = 0,
      .size = sizeof(PipelineMesh::PushConst),
  };

  VkPipelineLayoutCreateInfo layout_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &global_layout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &push_const_range,
  };
  VkResult res_ =
      vkCreatePipelineLayout(device, &layout_info, nullptr, &m_layout);
  XEV_ASSERT_VK(res_, "Failed to create pipeline layout");

  VkPipelineViewportStateCreateInfo viewport_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1,
  };

  VkPipelineColorBlendAttachmentState blend_attachments = {
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };

  VkPipelineColorBlendStateCreateInfo blend_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &blend_attachments,
  };

  VkVertexInputBindingDescription vertex_binding = {
      .binding = 0,
      .stride = sizeof(Mesh::Vertex),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };

  std::array<VkVertexInputAttributeDescription, 3> vertex_attributes = {{
      {.location = 0,
       .binding = 0,
       .format = VK_FORMAT_R32G32B32_SFLOAT,
       .offset = offsetof(Mesh::Vertex, position)},
      {.location = 1,
       .binding = 0,
       .format = VK_FORMAT_R32G32B32_SFLOAT,
       .offset = offsetof(Mesh::Vertex, normal)},
      {.location = 2,
       .binding = 0,
       .format = VK_FORMAT_R32G32_SFLOAT,
       .offset = offsetof(Mesh::Vertex, uv)},
  }};

  VkPipelineVertexInputStateCreateInfo vertex_input_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &vertex_binding,
      .vertexAttributeDescriptionCount =
          static_cast<uint32_t>(vertex_attributes.size()),
      .pVertexAttributeDescriptions = vertex_attributes.data(),
  };

  std::array<VkDynamicState, 2> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT,
                                                  VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamic_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
      .pDynamicStates = dynamic_states.data(),
  };

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
  };
  VkPipelineRasterizationStateCreateInfo rasterizer_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      // .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .lineWidth = 1.f,
  };
  VkPipelineMultisampleStateCreateInfo multisampling_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .sampleShadingEnable = VK_FALSE,
      .rasterizationSamples = sample_count,
      .minSampleShading = 1.0f,
  };
  VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_TRUE,
      .depthWriteEnable = VK_TRUE,
      .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
      .depthBoundsTestEnable = VK_FALSE,
      .stencilTestEnable = VK_FALSE,
      .minDepthBounds = 0.f,
      .maxDepthBounds = 1.f,
  };

  VkPipelineRenderingCreateInfo render_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &color_format,
      .depthAttachmentFormat = depth_format,
  };

  VkPipelineShaderStageCreateInfo vert_stage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = shader_vert,
      .pName = "main",
  };

  VkPipelineShaderStageCreateInfo frag_stage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = shader_frag,
      .pName = "main",
  };

  std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages = {
      vert_stage,
      frag_stage,
  };

  VkGraphicsPipelineCreateInfo pipeline_create_info = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = &render_info,
      .stageCount = (std::uint32_t)shader_stages.size(),
      .pStages = shader_stages.data(),
      .pVertexInputState = &vertex_input_state,
      .pInputAssemblyState = &input_assembly_state,
      .pViewportState = &viewport_state,
      .pRasterizationState = &rasterizer_state,
      .pMultisampleState = &multisampling_state,
      .pDepthStencilState = &depth_stencil_state,
      .pColorBlendState = &blend_state,
      .pDynamicState = &dynamic_info,
      .layout = m_layout,
  };

  res_ = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
                                   &pipeline_create_info, nullptr, &m_pipeline);
  XEV_ASSERT_VK(res_, "Failed to create graphics pipeline");

  vkDestroyShaderModule(device, shader_vert, nullptr);
  vkDestroyShaderModule(device, shader_frag, nullptr);
}

void PipelineMesh::destroy(VkDevice device) {
  vkDestroyPipelineLayout(device, m_layout, nullptr);
  vkDestroyPipeline(device, m_pipeline, nullptr);
}

void PipelineMesh::draw(VkCommandBuffer cmdbuf,
                        const Scene& scene,
                        const Camera& camera,
                        const std::vector<PipelineMesh::Command>& draw_cmds,
                        uint32_t width,
                        uint32_t height) {
  vkCmdBindPipeline(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

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
        cmdbuf, m_layout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
        sizeof(PushConst), &push_const);
    vkCmdDrawIndexed(cmdbuf, mesh.get_face_count() * 3, 1, 0, 0, 0);
  }
}

}  // namespace xev
