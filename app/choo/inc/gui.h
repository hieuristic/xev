#pragma once
#include <xev/renderer2D.h>
#include <xev/ui/font.h>

struct GUI {
  GUI(xev::Renderer2D& r2D_, xev::font& font_) : r2D(r2D_), font(font_) {};
  ~GUI();

  xev::Renderer2D& r2D;
  xev::Font& font;
};
