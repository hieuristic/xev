#pragma once
#include <xev/renderer2D.h>
#include <xev/ui/font.h>
#include <xev/ui/node.h>
#include <memory>

struct GUI {
  GUI(xev::Renderer2D& r2D_, xev::font& font_, float screenW, float screenH);
  ~GUI();

  std::unique_ptr<xev::ui::node> rootHauptmenu;

  xev::Renderer2D& r2D;
  xev::Font& font;
};
