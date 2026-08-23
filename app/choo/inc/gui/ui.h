#pragma once

namespace xev {
struct Renderer2D;
}

struct UI {
  virtual ~UI() = default;
  virtual void draw(xev::Renderer2D& r2D) = 0;
};
