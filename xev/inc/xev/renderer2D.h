#pragma once

namespace xev {

class Scene;

class Renderer2D : Renderer {
 public:
  Renderer2D(PipelineManager& pipeline_manager_,
             VkDescriptorSetLayout global_layout);
  ~Renderer2D();

  void draw(VkCommandBuffer cmdbuf,
            const Image& color_image,
            const GlobalDescriptorSet& desc_set,
            Color4<float> clear_color);

 private:
  void begin_render(VkCommandBuffer& cmdbuf,
                    const Image& color_image,
                    const Image& depth_image,
                    const Color4<float> clear_color);
  void end_render(VkCommandBuffer& cmdbuf);

  PipelineManager& m_pipeline_manager;
  PipelineMesh m_pipeline_2D;

  // This is in-sync with shaders/raster.slang
  struct PushConst {
    glm::mat4 transform;
    glm::vec2 uv_topleft;
    glm::vec2 uv_botright;
    uint32_t tex_id;
    uint32_t is_msdf;
  };
};

}  // namespace xev
