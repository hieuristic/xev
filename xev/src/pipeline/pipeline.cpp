#include <xev/logger.h>
#include <xev/pipeline.h>
#include <fstream>
#include <vector>

namespace xev {

static VkShaderModule Pipeline::load_shader_module(VkDevice device,
                                                   const char* path) const {
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

void Pipeline::create(const PipelineInfo& pipeinfo) {
  VkPushConstantRange push_const_range = {
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      .offset = 0,
      .size = pipeinfo.push_const_size,
  };
  VkPipelineLayoutCreateInfo layout_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &global_layout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &push_const_range,
  };
  VkResult res_ =
      vkCreatePipelineLayout(device, &layout_info, nullptr, &layout);
  XEV_ASSERT_VK(res_, "Failed to create pipeline layout");

  VkPipelineViewportStateCreateInfo viewport_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1,
  };

  VkPipelineColorBlendAttachmentState blend_attachments =
      pipeinfo.enable_blending
      ? {
            .blendEnable = VK_FALSE,
            .colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        } : {
    .blendEnable = VK_TRUE,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .colorBlendOp = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .alphaBlendOp = VK_BLEND_OP_ADD,
  };

  VkPipelineColorBlendStateCreateInfo blend_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &blend_attachments,
  };

  VkPipelineVertexInputStateCreateInfo vertex_input_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
  };

  std::vector<VkDynamicState> dynamic_states{VK_DYNAMIC_STATE_VIEWPORT,
                                             VK_DYNAMIC_STATE_SCISSOR};
  if (pipeinfo.dynamic_depth) {
    dynamic_states.push_back(VK_DYNAMIC_STATE_DEPTH);
  }

  VkPipelineDynamicStateCreateInfo dynamic_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
      .pDynamicStates = dynamic_states.data(),
  };
  VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = pipeinfo.topology,
      .primitiveRestartEnable = VK_FALSE,
  };
  VkPipelineRasterizationStateCreateInfo rasterizer_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .polygonMode = pipeinfo.polygon_mode,
      .cullMode = pipeinfo.cull_mode,
      .frontFace = pipeinfo.front_face,
      .lineWidth = 1.f,
  };
  VkPipelineMultisampleStateCreateInfo multisampling_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .sampleShadingEnable = VK_FALSE,
      .rasterizationSamples = pipeinfo.multisample_count,
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
  if (pipeinfo.enable_depth == false) {
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
  }
  VkPipelineRenderingCreateInfo render_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &pipeinfo.color_format,
      .depthAttachmentFormat = pipeinfo.depth_format,
  };
  VkPipelineShaderStageCreateInfo vert_stage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = pipeinfo.shader_vert,
      .pName = "main",
  };

  VkPipelineShaderStageCreateInfo frag_stage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = pipeinfo.shader_frag,
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
      .layout = layout,
  };

  res_ = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
                                   &pipeline_create_info, nullptr, &pipeline);
  XEV_ASSERT_VK(res_, "Failed to create graphics pipeline");
}

void Pipeline::destroy(VkDevice device) {
  vkDestroyPipelineLayout(device, layout, nullptr);
  vkDestroyPipeline(device, pipeline, nullptr);
}

}  // namespace xev
