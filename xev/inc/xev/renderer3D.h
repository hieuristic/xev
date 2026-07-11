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
            const Image& image,
            const Scene& scene,
            const Camera& camera,
            Color4<float> clear_color);
  void draw_mesh(VkCommandBuffer cmd,
                 const Scene& scene,
                 const Camera& camera,
                 uint32_t width,
                 uint32_t height);

 public:
  // in sync with scene.slang
  struct RenderScene {
    // camera
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 view_pos;

    // ambient
    glm::vec3 ambient_color;
    float ambient_intensity;

    // fog
    glm::vec3 fog_color;
    float fog_intensity;

    // light
    VkDeviceAddress light_ids;

    // material
    VkDeviceAddress material_ids;
  };

 private:
  void prepare_image(VkCommandBuffer& cmd, const Image& image);
  void prepare_transfer(VkCommandBuffer& cmd, const Image& image);
  void begin_render(VkCommandBuffer& cmd,
                    const Image& image,
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
