#pragma once
#include <xev/camera.h>
#include <xev/pipeline/pipeline.h>
#include <xev/resource/scene.h>

namespace xev {

class PipelineMesh : Pipeline {
 public:
  struct SceneUniform {};
  struct SceneMeshes {};
  struct SceneMaterials {};
  struct Command {
    uint32_t mesh_id;
    uint32_t material_id;
    glm::mat4 to_world;
    bool is_skinned;
    VkDeviceAddress skinned_mesh_address;

    bool operator<(const Command& other) const {
      return material_id < other.material_id;
    };
  };

 public:
  void create(const Backend& backend,
              VkFormat color_format,
              VkFormat depth_format,
              VkSampleCountFlagBits sample_count);
  void destroy(const Backend& backend);
  void draw(const Backend& backend,
            VkCommandBuffer cmdbuf,
            VkExtent2D ext,
            const Scene& scene,
            const Camera& camera,
            const std::vector<Command>& draw_cmds);

 private:
  struct PushConst {
    glm::mat4 mvp;
    VkDeviceAddress scene_uniform;
    VkDeviceAddress vertex_buffer;
    uint32_t material_id;
    uint32_t padding;
  };
};

}  // namespace xev
