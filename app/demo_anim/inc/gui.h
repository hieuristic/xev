#pragma once
#include <xev/renderer2D.h>
#include <xev/ui/font.h>
#include <xev/ui/layout.h>

enum struct GameState : uint8_t;

struct GUI {
  GUI(float screenW, float screenH, xev::Renderer2D& r2D_, xev::Font& font_)
      : layout(screenW, screenH, r2D_, font_) {}
  ~GUI() = default;

  void draw_hauptmenu(glm::vec2 mousePos,
                      bool isMouseDown,
                      GameState& state,
                      bool& isRunning);
  void draw_loading_screen();
  void draw_gameplay();

  xev::ui::Layout layout;
};
