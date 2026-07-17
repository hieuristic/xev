#pragma once
#include <xev/color.h>
#include <xev/pipeline/pipeline_mesh.h>
#include <xev/resource/image.h>

namespace xev {

class PipelineManager;

class Renderer3D {
 public:
  Renderer3D(PipelineManager& pipeline_manager_,
             VkDescriptorSetLayout global_layout);
  ~Renderer3D();

  void draw(VkCommandBuffer cmd,
            const Image& color_image,
            const Image& depth_image,
            const GlobalDescriptorSet& desc_set,
            const Scene& scene,
            const Camera& camera,
            Color4<float> clear_color);
  void draw_mesh(VkCommandBuffer cmd,
                 const Scene& scene,
                 const Camera& camera,
                 uint32_t width,
                 uint32_t height);

 private:
  void prepare_attachments(VkCommandBuffer& cmd,
                           const Image& color_image,
                           const Image& depth_image);
  void prepare_transfer(VkCommandBuffer& cmd, const Image& color_image);
  void begin_render(VkCommandBuffer& cmd,
                    const Image& color_image,
                    const Image& depth_image,
                    const Color4<float> clear_color);
  void end_render(VkCommandBuffer& cmd);

  PipelineManager& m_pipeline_manager;
  PipelineMesh m_pipeline_mesh;
  std::vector<PipelineMesh::Command> m_mesh_cmds;

 public:  // scene limit
  static const uint32_t MAX_LIGHTS = 1000;
  static const uint32_t MAX_SHADOW_LIGHTS = 3;
};

}  // namespace xev
