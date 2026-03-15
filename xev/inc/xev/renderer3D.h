#pragma once
#include <xev/image.h>
#include <xev/renderer.h>
#include <xev/pipelines/pipeline_mesh.h>

namespace xev {

class Renderer3D : Renderer {
 public:
  Renderer3D(std::shared_ptr<Backend> backend, ImageSize image_size);
  ~Renderer3D();

  void draw(VkCommandBuffer cmd, const Scene& scene);
  void draw_mesh(VkCommandBuffer cmd, const Scene& scene);
  void draw_mesh_skinned(VkCommandBuffer cmd, const Scene& scene);

 public:
  struct ImageSize {
    uint32_t width;
    uint32_t height;
  };

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
    gloat fog_intensity;

    // light
    VkDeviceAddress light_ids;

    // material
    VkDeviceAddress material_ids;
  };

 private:
  PipelineMesh m_pipeline_mesh;
  PipelineSkin m_pipeline_skin;

private:
  Image m_image;
}

}  // namespace xev
