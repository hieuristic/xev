#include "gui.h"
#include "game.h"

GUI::GUI(xev::Renderer2D& r2D_, xev::font& font_, ) : r2D(r2D_), font(font_) {}

void GUI::draw_hauptmenu(float screenW,
                         float screenH,
                         glm::vec2 mousePos,
                         bool isMouseDown,
                         GameState& state,
                         bool& isRuning) {
  ui_ctx.draw(screenW, screenH, mousePos, isMouseDown, [&] {
    ui.container(
        {.direction = xev::ui::Direction::Vertical,
         .padding = xev::Bound2(200.0f),
         .gap = 20.0f},
        [&] {
          ui.text("Test Game", 2.0f);
          if (ui.button("> Start Game", 1.0f)) state = STATE_GAMEPLAY;
          if (ui.button("> Quit", 1.0f, {})) isRunning = false;
        }, );
  });
}

void GUI::draw_gameplay() {
  ;  // TODO
}
