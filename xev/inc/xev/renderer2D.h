#pragma once

namespace xev {

class Renderer2D {
 public:
  Renderer2D(const Backend& backend);
  void draw();
};

}  // namespace xev
