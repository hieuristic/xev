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

  PipelineMesh() {
    {
      pipeInfo.shaderVertSrc = "mesh.spv";
      pipeInfo.shaderFragSrc = "mesh.spv";
      pipeInfo.pushConstSize = sizeof(PipelineMesh::PushConst);
      pipeInfo.enableBlending = false;
      pipeInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      pipeInfo.polygonMode = VK_POLYGON_MODE_FILL;
      pipeInfo.cullMode = VK_CULL_MODE_BACK_BIT;
      pipeInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
      pipeInfo.multisampleCount = VK_SAMPLE_COUNT_1_BIT;
      pipeInfo.enableDepth = true;
    };
  }

  void draw(VkCommandBuffer cmdbuf,
            const Scene& scene,
            const Camera& camera,
            const std::vector<DrawInfo>& drawInfos,
            uint32_t width,
            uint32_t height);
};

}  // namespace xev
