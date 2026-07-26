#pragma once
#include <xev/volk.h>
#include <xev/renderer.h>

namespace xev {

class Image;
class Scene;
class Camera;
class Color4;
class PipelineMesh;
class PipelineManager;

class Renderer3D : Renderer {
 public:
  Renderer3D(PipelineManager& pipeline_manager_,
             VkDescriptorSetLayout global_layout);
  ~Renderer3D();

  void draw(VkCommandBuffer cmdbuf,
            const Image& color_image,
            const Image& depth_image,
            const GlobalDescriptorSet& desc_set,
            const Scene& scene,
            const Camera& camera,
            Color4<float> clear_color);
  void draw_mesh(VkCommandBuffer cmdbuf,
                 const Scene& scene,
                 const Camera& camera,
                 uint32_t width,
                 uint32_t height);

  static const uint32_t MAX_LIGHTS = 1000;
  static const uint32_t MAX_SHADOW_LIGHTS = 3;

 private:
  void prepare_attachments(VkCommandBuffer& cmdbuf,
                           const Image& color_image,
                           const Image& depth_image);
  void prepare_transfer(VkCommandBuffer& cmdbuf, const Image& color_image);
  void begin_render(VkCommandBuffer& cmdbuf,
                    const Image& color_image,
                    const Image& depth_image,
                    const Color4<float> clear_color);
  void end_render(VkCommandBuffer& cmdbuf);

  PipelineManager& m_pipeline_manager;
  PipelineMesh m_pipeline_mesh;
  std::vector<PipelineMesh::Command> m_mesh_cmds;
};

}  // namespace xev
