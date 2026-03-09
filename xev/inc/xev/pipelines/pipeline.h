#pragma once

namespace xev {

class Pipeline {
public:
  virtual void draw();

private:
  VkPipelineLayout m_pipeline_layout;
  VkPipeline m_pipeline;
}

}
