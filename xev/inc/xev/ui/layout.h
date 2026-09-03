#pragma once
#include <concepts>
#include <glm/glm.hpp>
#include <vector>

#include <xev/renderer2D.h>
#include <xev/ui/font.h>
#include <xev/ui/element.h>

namespace xev {
namespace ui {

// Inspired by clay.h Tks :)

struct Layout {
  Layout(Renderer2D& r2D, Font& font) : m_r2D(r2D), m_font(font) {}
  void solve();
  void render();
  void print_tree_layout() const;

  template <typename F>
    requires std::invocable<F>
  void container(Element&& e, F&& cb) {
    e.type = ElementType::Container;
    uint32_t idx = push(std::move(e));
    m_parents.push_back(idx);
    cb();
    m_parents.pop_back();
  }

  template <typename F>
    requires std::invocable<F>
  void draw(float screenW,
            float screenH,
            glm::vec2 mousePos,
            bool isMouseDown,
            F&& cb) {
    m_mousePos = mousePos;
    m_isMouseDown = isMouseDown;
    m_elements.clear();
    m_parents.clear();

    container(
        Element{
            .style = {.direction = Direction::Vertical,
                      .sizing = {.type = SizingType::Fixed, .value = screenW}},
            .bound = Bound2(0.0, 0.0, screenW, screenH),
        },
        std::forward<F>(cb));

    solve();
    render();
  }

  void text(std::string_view label, float fontSize, Style&& style = Style{});
  bool button(std::string_view label,
              float fontSize,
              glm::vec3& color,
              Style&& style = Style{});

 private:
  uint32_t push(Element&& e);

  glm::vec2 m_mousePos;
  bool m_isMouseDown;

  std::vector<Element> m_elements;
  std::vector<uint32_t> m_parents;

  Renderer2D& m_r2D;
  Font& m_font;
};

}  // namespace ui
}  // namespace xev
