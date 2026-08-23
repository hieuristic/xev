#pragma once
#include <vector>
#include "gui/ui.h"

struct Hauptmenu : public UI {
  void init();
  void draw(xev::Renderer2D& r2D) override;

  std::vector<Button> btns;
};
