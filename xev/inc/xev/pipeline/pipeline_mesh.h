#pragma once
#include <xev/camera.h>
#include <xev/pipeline/pipeline.h>
#include <xev/resource/scene.h>

namespace xev {

class PipelineMesh : public Pipeline {
 public:
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
  void create(VkDevice device,
              VkFormat color_format,
              VkFormat depth_format,
              VkDescriptorSetLayout global_layout,
              VkSampleCountFlagBits sample_count);
  void destroy(VkDevice device);
  void draw(VkCommandBuffer cmdbuf,
            const Scene& scene,
            const Camera& camera,
            const std::vector<Command>& draw_cmds,
            uint32_t width,
            uint32_t height);

 private:
  // This is in-sync with shaders/mesh.slang
  struct PushConst {
    glm::mat4 view_proj;
    glm::mat4 model_mat;
    glm::vec3 cam_xyz;
    VkDeviceAddress scene_addr;
    VkDeviceAddress vert_addr;
    uint32_t mat_id;
    uint32_t padding;
  };
};

}  // namespace xev
