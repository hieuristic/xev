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

  uint32_t upIdx{NULLIDX};    // to parents
  uint32_t downIdx{NULLIDX};  // to first child
  uint32_t sideIdx{NULLIDX};  // to sibling

  std::string_view textData{};
  float fontSize{1.0f};
};

struct Context {
  Context(Renderer2D& r2D, Font& font);
  void measure();
  void solve();

  template <typename F>
    requires std::invocable<F>
  void container(Element e, F&& cb) {
    e.type = ElementType::Container;
    uint32_t idx = push(e);
    m_bfs.push_back(idx);
    cb();
    pop();
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

  void solve();
  void render()

  void text(std::string_view label, float fontSize);
  bool button(std::string_view label, float fontSize);

 private:
  uint32_t push(const Element& e);
  void pop();

  glm::vec2 m_mousePos;
  bool m_isMouseDown;

  std::vector<Element> m_elements;
  std::vector<uint32_t> m_parents;

  Renderer2D& m_r2D;
  Font& m_font;
};

}  // namespace ui
}  // namespace xev
