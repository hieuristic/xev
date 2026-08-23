#pragma once
#include "gui/ui.h"

namespace ui {

struct Node {
  virtual void draw(xev::Renderer2D& r2D) = 0;

  template <std::derived_from<UI> T>
  void add_child(T uiKind) {
  };

  vector<UI*> kinder;
}

}  // namespace ui
