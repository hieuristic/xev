#include "gui.h"
#include "game.h"

void GUI::draw_hauptmenu(float screenW,
                         float screenH,
                         glm::vec2 mousePos,
                         bool isMouseDown,
                         GameState& state,
                         bool& isRunning) {
  glm::vec3 btnColor{0.5, 1.0, 0.5};
  glm::vec3 btnColor2{1.0, 0.5, 0.5};
  ui.draw(screenW, screenH, mousePos, isMouseDown, [&] {
    ui.container(
        xev::ui::Element{
            .style = {.direction = xev::ui::Direction::Vertical,
                      .padding = xev::Bound2(50.0f),
                      .gap = 20.0f},
        },
        [&] {
          ui.text("Test Game", 2.0f);
          ui.button("> Start Game", 1.0f, btnColor,
                    {.sizing = {.type = xev::ui::SizingType::Grow}},
                    [&] { state = GameState::Gameplay; });
          ui.button("> Quit", 1.0f, btnColor2,
                    {.sizing = {.type = xev::ui::SizingType::Grow}},
                    [&] { isRunning = false; });
        });
  });
  // ui.print_tree_layout();
  // exit(1);
}

void GUI::draw_gameplay() {
  ;  // TODO
}
