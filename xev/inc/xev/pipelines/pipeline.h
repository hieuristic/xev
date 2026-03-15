#pragma once

namespace xev {

class Pipeline {
 public:
  virtual void draw();

 private:
  VkPipelineLayout m_layout;
  VkPipeline m_pipeline;
}

}  // namespace xev
