#pragma once
#include <xev/renderer2D.h>
#include <xev/ui/layout.h>
#include <xev/ui/font.h>

enum struct GameState : uint8_t;

struct GUI {
  GUI(xev::Renderer2D& r2D_, xev::Font& font_) : ui(r2D_, font_) {}
  ~GUI() = default;

  void draw_hauptmenu(float screenW,
                      float screenH,
                      glm::vec2 mousePos,
                      bool isMouseDown,
                      GameState& state,
                      bool& isRunning);
  void draw_gameplay();

  xev::ui::Context ui;
};
