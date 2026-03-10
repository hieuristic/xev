#pragma once
#include <xev/renderer.h>

namespace xev {

class Renderer2D : Renderer {
public:
  Renderer2D(const Backend &backend);
  void draw();
};

} // namespace xev
