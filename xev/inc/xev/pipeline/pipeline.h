#pragma once
#include <xev/resource/resource.h>
#include <xev/volk.h>

namespace xev {

class Pipeline {
 public:
  virtual void draw() {}

 protected:
  VkPipelineLayout m_layout;
  VkPipeline m_pipeline;
};

}  // namespace xev
