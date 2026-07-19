#pragma once

namespace xev {

class PipelineRaster : Pipeline {
 public:
  void create(VkDevice device,
              VkFormat color_format,
              VkDescriptorSetLayout global_layout)
  void destroy(VkDevice);
  void draw(VkCommandBuffer cmdbuf,
            const std::vector<std::pair<Raster, uint32_t>>& rasters,
            uint32_t width,
            uint32_t height);
  struct PushConst
};

}  // namespace xev
