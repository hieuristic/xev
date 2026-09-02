#include "gui.h"
#include "game.h"

void GUI::draw_hauptmenu(float screenW,
                         float screenH,
                         glm::vec2 mousePos,
                         bool isMouseDown,
                         GameState& state,
                         bool& isRunning) {
  glm::vec3 btnColor{1.0, 0.0, 1.0};
  ui.draw(screenW, screenH, mousePos, isMouseDown, [&] {
    ui.container(
        xev::ui::Element{
            .style = {.direction = xev::ui::Direction::Vertical,
                      .padding = xev::Bound2(200.0f),
                      .gap = 20.0f},
        },
        [&] {
          ui.text("Test Game", 2.0f);
          if (ui.button("> Start Game", 1.0f, btnColor))
            state = GameState::Gameplay;
          if (ui.button("> Quit", 1.0f, btnColor)) isRunning = false;
        });
  });
}

void GUI::draw_gameplay() {
  ;  // TODO
}
