#pragma once
#include <xev/color.h>
#include <xev/pipeline/pipeline_raster.h>
#include <xev/renderer.h>
#include <xev/volk.h>
#include <glm/glm.hpp>
#include <string>

namespace xev {

struct Font;
struct Image;
struct Buffer;
struct PipelineManager;
struct GlobalDescriptorSet;

struct Renderer2D : public Renderer {
  Renderer2D(PipelineManager& pipelineManager,
             ResourceManager& resourceManager,
             uint32_t numFrameInFlight);
  ~Renderer2D();

  void draw(VkCommandBuffer cmdBuf,
            const Image& colorImage,
            const GlobalDescriptorSet& descSet,
            uint32_t currFrameIdx,
            const Color4<float>& clearColor);
  void draw_text(const Font& font, std::string_view text, glm::mat4 transform);
  void draw_image(glm::mat4 transform, glm::vec4 uvBounds, uint32_t texID);
  void draw_rect(glm::mat4 transform);

  static const uint32_t MAX_DRAW_CALLS = 1000;

 private:
  void begin_render(VkCommandBuffer cmdBuf,
                    const Image& colorImage,
                    const Color4<float>& clearColor);

  ResourceManager& m_resourceManager;
  PipelineManager& m_pipelineManager;
  PipelineRaster m_pipelineRaster;

  std::vector<Buffer> m_drawInfoBuffers;
  std::vector<PipelineRaster::DrawInfo> m_drawInfos;
};

}  // namespace xev
