#pragma once
#include <xev/renderer.h>
#include <xev/volk.h>

namespace xev {

class Image;
class Buffer;
class Color4;
class PipelineRaster;
class PipelineManager;
class GlobalDescriptorSet;

class Renderer2D : Renderer {
 public:
  Renderer2D(PipelineManager& pipelineManager,
             ResourceManager& resourceManager,
             VkDescriptorSetLayout descSetLayout,
             uint32_t numFrameInFlight);
  ~Renderer2D();

  void draw(VkCommandBuffer cmdBuf,
            const Image& colorImage,
            const GlobalDescriptorSet& descSet,
            uint32_t currFrameIdx,
            Color4<float> clearColor);
  void draw_text(Font font,
                 std::string text,
                 glm::mat4 transform,
                 uint32_t lineWidth);
  void draw_image(uint32_t tex_id,
                  glm::mat4 transform,
                  glm::vec2 uv_topLeft,
                  glm::vec2 uv_botRight);

  static const uint32_t MAX_DRAW_CALLS = 1000;

 private:
  void begin_render(VkCommandBuffer& cmdBuf,
                    const Image& colorImage,
                    const Image& depthImage,
                    const Color4<float> clearColor);
  void end_render(VkCommandBuffer& cmdBuf);

  ResourceManager& m_resourceManager;
  PipelineManager& m_pipelineManager;
  PipelineRaster m_pipelineRaster;
  VkDescriptorSetLayout m_descSetLayout;

  std::vector<Buffer> m_drawInfoBuffers;
  std::vector<PipelineRaster::DrawInfo> m_drawInfos;
};

}  // namespace xev
