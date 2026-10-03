#pragma once
#include <xev/pipeline/pipeline.h>
#include <glm/glm.hpp>

namespace xev {
struct Scene;
struct Camera;
}  // namespace xev

namespace xev::pipe {

struct Mesh : public RenderPipeline {
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
    uint32_t meshId;
    uint32_t materialId;
    glm::mat4 toWorld;
    bool isSkinned;
    VkDeviceAddress skinnedMeshAddress;

    bool operator<(const DrawInfo& other) const {
      return materialId < other.materialId;
    };
  };

  Mesh() {
    {
      info.shaderVertSrc = "mesh.spv";
      info.shaderFragSrc = "mesh.spv";
      info.pushConstSize = sizeof(Mesh::PushConst);
      info.enableBlending = false;
      info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      info.polygonMode = VK_POLYGON_MODE_FILL;
      info.cullMode = VK_CULL_MODE_BACK_BIT;
      info.frontFace = VK_FRONT_FACE_CLOCKWISE;
      info.multisampleCount = VK_SAMPLE_COUNT_1_BIT;
      info.enableDepth = true;
    };
  }

  void draw(VkCommandBuffer cmdbuf,
            const Scene& scene,
            const Camera& camera,
            const std::vector<DrawInfo>& drawInfos,
            uint32_t width,
            uint32_t height);
};

}  // namespace xev::pipe
