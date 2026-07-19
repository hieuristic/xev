#include <xev/pipeline_raster.h>

void PipelineRaster::create(VkDevice device,
                            VkFormat color_format,
                            VkDescriptorSetLayout global_layout) {
  VkShaderModule shader_vert, shader_frag;
  const char* base_path = SDL_GetBasePath();
  std::string shader_path =
      base_path ? std::string(base_path) + "../shaders/raster.spv"
                : "shaders/raster.spv";

  shader_vert = load_shader_module(device, shader_path.c_str());
  shader_frag = load_shader_module(device, shader_path.c_str());

  PipelineInfo pipeinfo{
      .shader_vert = shader_vert,
      .shader_frag = shader_frag,
      .push_const_size = sizeof(PipelineRaster::PushConst),
      .enable_blending = true,
      .topology = VK_PRIMITIV_TOPOLOGY_TRIANGLE_LIST,
      .polygon_mode = VK_POLYGON_MODE_FULL,
      .cull_mode = VK_CULL_MODE_NONE,
      .color_format = color_format,
      .multisample_count = VK_SAMPLE_COUNT_1_BIT,
      .enable_depth = false,
  };
  create(pipeinfo);

  vkDestroyShaderModule(device, shader_vert, nullptr);
  vkDestroyShaderModule(device, shader_frag, nullptr);
};

void PipelineRaster::draw(
    VkCommandBuffer cmdbuf,
    const std::vector<std::pair<Raster, uint32_t>>& rasters,
    uint32_t width,
    uint32_t height) {
  vkCmdBindPipeline(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}
}
