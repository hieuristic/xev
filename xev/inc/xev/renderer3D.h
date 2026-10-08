#pragma once
#include <xev/color.h>
#include <xev/frame_context.h>
#include <xev/pipeline/mesh.h>
#include <xev/renderer.h>
#include <xev/resource/buffer_array.h>
#include <xev/volk.h>

namespace xev {

struct Image;
struct Scene;
struct Camera;
struct PipelineManager;

struct Renderer3D : public Renderer {
  Renderer3D(PipelineManager& pipelineManager,
             ResourceManager& pipelineManager,
             uint32_t numFrameInFlight);
  ~Renderer3D();

  void draw(VkCommandBuffer cmdbuf,
            const Image& colorImage,
            const Image& depthImage,
            const GlobalDescriptorSet& desc_set,
            const Scene& scene,
            const Camera& camera,
            const Color4<float>& clearColor,
            const Buffer& boneTransform,
            const uint32_t currFrameIdx = 0);
  void draw_mesh(VkCommandBuffer cmdbuf,
                 const Scene& scene,
                 const Camera& camera,
                 const uint32_t width,
                 const uint32_t height,
                 const uint32_t currFrameIdx,
                 const bool useSkinned = false);

  void prepare_render(VkCommandBuffer& cmdbuf,
                      const Image& color_image,
                      const Image& depth_image,
                      const Color4<float> clear_color);

  void upload_skinning(uint32_t currFrameIdx, std::span<const glm::mat4> data);
  void dispatch_skinning(VkCommandBuffer cmdbuf);
  void wait_skinning(VkCommandBuffer cmdbuf);

  void prepare_visibles();

  static constexpr uint32_t MAX_LIGHTS = 1000;
  static constexpr uint32_t MAX_SHADOW_LIGHTS = 3;
  static constexpr uint32_t MAX_BONES = 256;
  static constexpr uint32_t MAX_ANIMATION_INSTANCE = 16;

 private:
  PipelineManager& m_pipelineManager;
  ResourceManager& m_resourceManager;

  pipe::Mesh m_pipeMesh;
  std::vector<pipe::Mesh::DrawInfo> m_meshInfos;

  pipe::Skinning m_pipeSkinning;
  std::vector<pipe::Mesh::DispatchInfo> m_skinningInfos;
  BufferArray m_bufArrBoneTransforms{
      FrameContext::MAX_IN_FLIGHT, MAX_BONES * sizeof(glm::mat4),
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
      VMA_MEMORY_USAGE_AUTO};
  std::array<uint32_t, FrameContext::MAX_IN_FLIGHT> m_numBones{};
};

}  // namespace xev
