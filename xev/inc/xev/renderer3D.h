#pragma once
#include <xev/color.h>
#include <xev/pipeline/mesh.h>
#include <xev/renderer.h>
#include <xev/volk.h>

namespace xev {

struct Image;
struct Scene;
struct Camera;
struct PipelineManager;

struct Renderer3D : public Renderer {
  Renderer3D(PipelineManager& manager);
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
  void dispatch_skinning(VkCommandBuffer);

  void prepare_render(VkCommandBuffer& cmdbuf,
                      const Image& color_image,
                      const Image& depth_image,
                      const Color4<float> clear_color);
  void prepare_skinning();
  void prepare_visibles();

  static const uint32_t MAX_LIGHTS = 1000;
  static const uint32_t MAX_SHADOW_LIGHTS = 3;

 private:
  PipelineManager& m_pipelineManager;
  pipe::Mesh m_pipeMesh;
  pipe::Skinning m_pipeSkinning;
  std::vector<pipe::Mesh::DrawInfo> m_meshInfos;
  std::vector<pipe::Mesh::DispatchInfo> m_S
};

}  // namespace xev
