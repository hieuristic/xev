#include "gui.h"
#include <xev/logger.h>
#include "game.h"

void GUI::draw_hauptmenu(glm::vec2 mousePos,
                         bool isMouseDown,
                         GameState& state,
                         bool& isRunning) {
  glm::vec3 btnColor{0.5, 0.5, 0.5};
  glm::vec3 btnColor2{1.0, 0.5, 0.5};
  layout.draw(mousePos, isMouseDown, [&] {
    layout.container(
        xev::ui::Element{
            .style = {.direction = xev::ui::Direction::Vertical,
                      .padding = xev::Bound2(50.0f),
                      .gap = 20.0f},
        },
        [&] {
          layout.text("Goffy Ahh", 2.0f);
          layout.button(
              "> Start", 1.0f, btnColor,
              {
                  .sizing = {.type = xev::ui::SizingType::Grow},
                  .padding = xev::Bound2(20.0f, 10.0f),
              },
              [&] { state = GameState::Gameplay; },
              [&](xev::ui::Element& el) { el.color = btnColor2; });
          layout.button(
              "> Quit", 1.0f, btnColor,
              {
                  .sizing = {.type = xev::ui::SizingType::Grow},
                  .padding = xev::Bound2(20.0f, 10.0f),
              },
              [&] { isRunning = false; },
              [&](xev::ui::Element& el) { el.color = btnColor2; });
        });
  });
  // layout.print_tree_layout();
  // exit(1);
}

void GUI::draw_gameplay() {
  ;  // TODO
}
