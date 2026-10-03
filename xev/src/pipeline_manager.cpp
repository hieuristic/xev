#include <xev/filesystem/fs.h>
#include <xev/logger.h>
#include <xev/pipeline/mesh.h>
#include <xev/pipeline/pipeline.h>
#include <xev/pipeline_manager.h>
#include <fstream>
#include <vector>

namespace xev {

void PipelineManager::create(RenderPipeline& pipe) {
  VkPushConstantRange pushConstRange = {
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      .offset = 0,
      .size = pipe.info.pushConstSize,
  };
  VkPipelineLayoutCreateInfo layoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &m_descSetLayout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstRange,
  };
  VkResult res_ =
      vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &pipe.layout);
  XEV_ASSERT_VK(res_, "Failed to create pipeline layout");

  VkPipelineViewportStateCreateInfo viewportState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1,
  };

  VkPipelineColorBlendAttachmentState blendAttachments;
  if (!pipe.info.enableBlending) {
    blendAttachments = {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
  } else {
    blendAttachments = {
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
  }

  VkPipelineColorBlendStateCreateInfo blendState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &blendAttachments,
  };

  VkPipelineVertexInputStateCreateInfo vertexInputState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
  };

  std::vector<VkDynamicState> dynamicStates{VK_DYNAMIC_STATE_VIEWPORT,
                                            VK_DYNAMIC_STATE_SCISSOR};
  if (pipe.info.dynamicDepth) {
    dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
  }

  VkPipelineDynamicStateCreateInfo dynamicInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data(),
  };
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = pipe.info.topology,
      .primitiveRestartEnable = VK_FALSE,
  };
  VkPipelineRasterizationStateCreateInfo rasterizerState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .polygonMode = pipe.info.polygonMode,
      .cullMode = pipe.info.cullMode,
      .frontFace = pipe.info.frontFace,
      .lineWidth = 1.f,
  };
  VkPipelineMultisampleStateCreateInfo multisamplingState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .sampleShadingEnable = VK_FALSE,
      .rasterizationSamples = pipe.info.multisampleCount,
      .minSampleShading = 1.0f,
  };
  VkPipelineDepthStencilStateCreateInfo depthStencilState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_TRUE,
      .depthWriteEnable = VK_TRUE,
      .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
      .depthBoundsTestEnable = VK_FALSE,
      .stencilTestEnable = VK_FALSE,
      .minDepthBounds = 0.f,
      .maxDepthBounds = 1.f,
  };
  if (pipe.info.enableDepth == false) {
    depthStencilState.depthTestEnable = VK_FALSE;
    depthStencilState.depthWriteEnable = VK_FALSE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_NEVER;
  }
  VkPipelineRenderingCreateInfo renderInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &pipe.info.colorFormat,
      .depthAttachmentFormat = pipe.info.depthFormat,
  };

  VkShaderModule shaderVert, shaderFrag;
  XEV_INFO("pipe vert src: {}", pipe.info.shaderVertSrc);
  XEV_INFO("enable depth: {}", pipe.info.enableDepth);
  load_shader(shaderVert, pipe.info.shaderVertSrc);
  load_shader(shaderFrag, pipe.info.shaderFragSrc);
  VkPipelineShaderStageCreateInfo vertStage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = shaderVert,
      .pName = "main",
  };

  VkPipelineShaderStageCreateInfo fragStage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = shaderFrag,
      .pName = "main",
  };
  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
      vertStage,
      fragStage,
  };

  VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = &renderInfo,
      .stageCount = (std::uint32_t)shaderStages.size(),
      .pStages = shaderStages.data(),
      .pVertexInputState = &vertexInputState,
      .pInputAssemblyState = &inputAssemblyState,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizerState,
      .pMultisampleState = &multisamplingState,
      .pDepthStencilState = &depthStencilState,
      .pColorBlendState = &blendState,
      .pDynamicState = &dynamicInfo,
      .layout = pipe.layout,
  };

  res_ =
      vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1,
                                &pipelineCreateInfo, nullptr, &pipe.pipeline);
  XEV_ASSERT_VK(res_, "Failed to create render pipeline");

  vkDestroyShaderModule(m_device, shaderVert, nullptr);
  vkDestroyShaderModule(m_device, shaderFrag, nullptr);
}

void PipelineManager::destroy(RenderPipeline& pipe) const {
  vkDestroyPipelineLayout(m_device, pipe.layout, nullptr);
  vkDestroyPipeline(m_device, pipe.pipeline, nullptr);
}

void PipelineManager::create(ComputePipeline& pipe) {
  VkPushConstantRange pushConstRange = {
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      .offset = 0,
      .size = pipe.info.pushConstSize,
  };
  VkPipelineLayoutCreateInfo layoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &m_descSetLayout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstRange,
  };
  VkResult res_ =
      vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &pipe.layout);
  XEV_ASSERT_VK(res_, "Failed to create pipeline layout");

  VkShaderModule shader;
  load_shader(shader, pipe.info.shaderSrc);

  VkPipelineShaderStageCreateInfo stage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = shader,
      .pName = "main",
  };

  VkComputePipelineCreateInfo pipeInfo = {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = stage,
      .layout = pipe.layout,
  };

  res_ = vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipeInfo,
                                  nullptr, &pipe.pipeline);
  XEV_ASSERT_VK(res_, "Failed to create compute pipeline");

  vkDestroyShaderModule(m_device, shader, nullptr);
}

void PipelineManager::destroy(ComputePipeline& pipe) const {
  vkDestroyPipelineLayout(m_device, pipe.layout, nullptr);
  vkDestroyPipeline(m_device, pipe.pipeline, nullptr);
}

void PipelineManager::load_shader(VkShaderModule& mod, std::string path) const {
  XEV_INFO("Input path: {}", path);

  std::vector<uint8_t> shader_src_ = m_fileSys.read(path);

  VkResult res_;
  VkShaderModuleCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = shader_src_.size(),
      .pCode = reinterpret_cast<const uint32_t*>(shader_src_.data()),
  };

  res_ = vkCreateShaderModule(m_device, &info, nullptr, &mod);
  XEV_ASSERT_VK(res_, "Failed to load shader module");
}

}  // namespace xev
