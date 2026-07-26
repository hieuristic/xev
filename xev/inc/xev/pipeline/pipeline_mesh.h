#pragma once
#include <xev/camera.h>
#include <xev/pipeline/pipeline.h>
#include <xev/resource/scene.h>

namespace xev {

class PipelineMesh : public Pipeline {
 public:
  // This is in-sync with shaders/mesh.slang
  struct PushConst {
    glm::mat4 viewProj;
    glm::mat4 modelMat;
    glm::vec3 camXYZ;
    VkDeviceAddress sceneBuffer;
    VkDeviceAddress vertexBuffer;
    uint32_t matID;
    uint32_t padding;
  };

  PipelineInfo pipeInfo{
      .shaderVertSrc = "mesh.spv",
      .shaderFragSrc = "mesh.spv",
      .push_const_size = sizeof(PipelineMesh::PushConst),
      .enable_blending = false,
      .topology = VK_PRIMITIV_TOPOLOGY_TRIANGLE_LIST,
      .polygon_mode = VK_POLYGON_MODE_FULL,
      .cull_mode = VK_CULL_MODE_BACK_BIT,
      .front_face = VK_FRONT_FACE_CLOCKWISE,
      .colorFormat = colorFormat,
      .depthFormat = depthFormat,
      .multisample_count = VK_SAMPLE_COUNT_1_BIT,
      .enable_depth = true,
  };

  struct DrawInfo {
    uint32_t mesh_id;
    uint32_t material_id;
    glm::mat4 to_world;
    bool is_skinned;
    VkDeviceAddress skinned_mesh_address;

    bool operator<(const DrawInfo& other) const {
      return material_id < other.material_id;
    };
  };

  void draw(VkCommandBuffer cmdbuf,
            const Scene& scene,
            const Camera& camera,
            const std::vector<DrawInfo>& drawInfos,
            uint32_t width,
            uint32_t height);
};

}  // namespace xev
