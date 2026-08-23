#pragma once
#include <string>
#include "gui/ui.h"

namespace ui {

struct Button : public Node {
  void draw(xev::Renderer2D& r2D) override;

  glm::mat4 transform;
  std::string text;
};

}
