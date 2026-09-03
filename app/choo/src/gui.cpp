#include "gui.h"
#include <xev/logger.h>
#include "game.h"

void GUI::draw_hauptmenu(float screenW,
                         float screenH,
                         glm::vec2 mousePos,
                         bool isMouseDown,
                         GameState& state,
                         bool& isRunning) {
  glm::vec3 btnColor{0.5, 1.0, 0.5};
  glm::vec3 btnColor2{1.0, 0.5, 0.5};
  layout.draw(screenW, screenH, mousePos, isMouseDown, [&] {
    layout.container(
        xev::ui::Element{
            .style = {.direction = xev::ui::Direction::Vertical,
                      .padding = xev::Bound2(50.0f),
                      .gap = 20.0f},
        },
        [&] {
          layout.text("Test Game", 2.0f);
          layout.button(
              "> Start Game", 1.0f, btnColor,
              {.sizing = {.type = xev::ui::SizingType::Grow}},
              [&] {
                state = GameState::Gameplay;
                XEV_INFO("CLICKED!!");
              },
              [] { XEV_INFO("hovered!!"); });
          layout.button("> Quit", 1.0f, btnColor2,
                        {.sizing = {.type = xev::ui::SizingType::Grow}},
                        [&] { isRunning = false; });
        });
  });
  // layout.print_tree_layout();
  // exit(1);
}

void GUI::draw_gameplay() {
  ;  // TODO
}
