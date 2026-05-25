#pragma once
#include <xev/pipeline/pipeline_mesh.h>
#include <xev/resource/image.h>

namespace xev {

class Renderer3D {
 public:
  Renderer3D(PipelineManager& pipeline_manager_);
  ~Renderer3D();

  void draw(VkCommandBuffer cmd,
            const Image& image,
            const Scene& scene,
            const Camera& camera,
            const FrameArg& arg);
  void draw_mesh(VkCommandBuffer cmd, const Scene& scene, const Camera& camera);

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
  std::shared_ptr<Backend> m_backend;
  PipelineMesh m_pipeline_mesh;
  std::vector<PipelineMesh::Command> m_mesh_cmds;

 public:  // scene limit
  static const uint32_t MAX_LIGHTS = 1000;
  static const uint32_t MAX_SHADOW_LIGHTS = 3;
};

}  // namespace xev
