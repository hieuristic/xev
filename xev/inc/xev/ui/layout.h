#pragma once
#include <concepts>
#include <vector>

#include <xev/renderer2D.h>
#include <xev/ui/element.h>
#include <xev/ui/font.h>

namespace xev {
namespace ui {

// Inspired by clay.h Tks :)

struct Layout {
  Layout(float screenW, float screenH, Renderer2D& r2D, Font& font)
      : m_screenW(screenW), m_screenH(screenH), m_r2D(r2D), m_font(font) {}
  void draw(glm::vec2 mousePos, bool isMouseDown, std::function<void()> cb);
  void solve();
  void render();
  void set_resolution(float screenW, float screenH);
  void print_tree_layout() const;

  void container(Element&& e, std::function<void()> cb);
  void text(std::string_view label, float fontSize, Style&& style = Style{});
  void button(
      std::string_view label,
      float fontSize,
      glm::vec3& color,
      Style&& style = Style{},
      std::function<void()> onClick = [] {},
      std::function<void(Element&)> onHover = [](Element&){});

 private:
  uint32_t push(Element&& e);

  glm::vec2 m_mousePos;
  bool m_isMouseDown;
  float m_screenW{800.0f};
  float m_screenH{600.0f};

  std::vector<Element> m_elements;
  std::vector<uint32_t> m_parents;

  Renderer2D& m_r2D;
  Font& m_font;
};

}  // namespace ui
}  // namespace xev
