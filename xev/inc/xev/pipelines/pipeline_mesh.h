#pragma once
#include <xev/pipeline.h>

namespace xev {

class PipelineMesh : Pipeline {
 public:
  PipelineMesh(const Backend& backend);
  ~PipelineMesh();
}

}  // namespace xev
