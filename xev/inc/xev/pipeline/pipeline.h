#pragma once
#include <xev/resource/resource.h>
#include <xev/volk.h>

namespace xev {

class Pipeline {
 public:
  virtual void draw() {}
  VkPipelineLayout layout;
  VkPipeline pipeline;
};

}  // namespace xev
