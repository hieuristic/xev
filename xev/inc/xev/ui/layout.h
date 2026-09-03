#pragma once
#include <concepts>
#include <cstdint>
#include <glm/glm.hpp>
#include <string_view>
#include <vector>

#include <xev/renderer2D.h>
#include <xev/ui/font.h>
#include <xev/ui/style.h>

namespace xev {
namespace ui {

// Inspired by clay.h Tks :)

enum struct ElementType : uint8_t {
  Container,
  Text,
  Button,
};

constexpr uint32_t NULLIDX = UINT32_MAX;

struct Element {
  ElementType type{ElementType::Container};
  Style style{};
  Bound2 bound{};
  glm::vec2 size{};
  glm::vec2 cursor{};
  glm::vec3 color{};

  uint32_t parentIdx{NULLIDX};  // to parents
  uint32_t numChildren = 0;
  uint32_t numGrowChildren = 0;

  std::string_view textData{};
  float fontSize{1.0f};
};

struct Context {
  Context(Renderer2D& r2D, Font& font) : m_r2D(r2D), m_font(font) {}
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
