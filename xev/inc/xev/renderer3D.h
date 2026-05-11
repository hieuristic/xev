#pragma once
#include <xev/pipeline/pipeline_mesh.h>
#include <xev/resource/image.h>

namespace xev {

class Renderer3D {
 public:
  Renderer3D(std::shared_ptr<Backend> backend, uint32_t width, uint32_t height);
  ~Renderer3D();

  const Image& draw(VkCommandBuffer cmd,
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

 private:
  Image m_image{VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                             VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                             VK_IMAGE_USAGE_SAMPLED_BIT};
  Image m_depth_image{VK_FORMAT_D32_SFLOAT,
                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT};

 private:
  VkSemaphore m_sem_image = VK_NULL_HANDLE;
  VkSemaphore m_sem_drawn = VK_NULL_HANDLE;
  VkFence m_fence_inflight;

 public:  // scene limit
  static const uint32_t MAX_LIGHTS = 1000;
  static const uint32_t MAX_SHADOW_LIGHTS = 3;
};

}  // namespace xev
