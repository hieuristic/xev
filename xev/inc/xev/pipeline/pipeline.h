#pragma once
#include <xev/resource/resource.h>
#include <xev/resource_manager.h>
#include <xev/volk.h>

namespace xev {

class Pipeline {
 public:
  virtual void draw() {}
  virtual void get_pipeline_info(PipelineInfo& pipeInfo) const;
  VkPipelineLayout layout;
  VkPipeline pipeline;
};

}  // namespace xev
