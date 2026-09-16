#include <xev/logger.h>

#include "character.h"
#include "game.h"
#include "gui.h"

void GUI::draw_hauptmenu(glm::vec2 mousePos,
                         bool isMouseDown,
                         GameState& state,
                         bool& isRunning,
                         CharacterType& character) {
  glm::vec3 btnColor{0.5, 0.5, 0.5};
  glm::vec3 activeColor{0.2, 0.8, 0.4};
  glm::vec3 btnColor2{1.0, 0.5, 0.5};
  layout.draw(mousePos, isMouseDown, [&] {
    layout.container(
        xev::ui::Element{
            .style = {.direction = xev::ui::Direction::Vertical,
                      .padding = xev::Bound2(50.0f),
                      .gap = 20.0f},
        },
        [&] {
          layout.text("Game :)", 2.0f);
          layout.button(
              "> Start as Hieu", 1.0f, btnColor,
              {
                  .sizing = {.type = xev::ui::SizingType::Grow},
                  .padding = xev::Bound2(20.0f, 10.0f),
              },
              [&] {
                state = GameState::Loading;
                character = CharacterType::Hieu;
              },
              [&](xev::ui::Element& el) { el.color = btnColor2; });
          layout.button(
              "> Start as Ngok", 1.0f, btnColor,
              {
                  .sizing = {.type = xev::ui::SizingType::Grow},
                  .padding = xev::Bound2(20.0f, 10.0f),
              },
              [&] {
                state = GameState::Loading;
                character = CharacterType::Ngok;
              },
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

void GUI::draw_loading_screen() {
  layout.draw(glm::vec2(0.0f), false, [&] {
    layout.container(
        xev::ui::Element{
            .style = {.direction = xev::ui::Direction::Horizontal,
                      .padding = xev::Bound2(200.0f),
                      .gap = 20.0f},
        },
        [&] { layout.text("Loading...", 2.0f); });
  });
}

void GUI::draw_gameplay() {
  layout.draw(glm::vec2(0.0f), false, [&] {
    layout.container(
        xev::ui::Element{
            .style = {.direction = xev::ui::Direction::Horizontal,
                      .padding = xev::Bound2(20.0f),
                      .gap = 20.0f},
        },
        [&] {
          layout.text("health:", 1.0f);
          layout.text("100", 1.0f);
        });
  });
}
