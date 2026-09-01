#include <xev/ui/context.h>
#include <algorithm>

namespace xev {
namespace ui {

void Context::solve() {
  if (m_elements.empty()) return;

  // 1. aggregate children size to parent
  for (size_t i = m_elements.size(); i--;) {
    auto& el = m_element[i];
    if (el.numChildren > 0) {
      if (el.style.direction == Direction::Vertical) {
        el.size.y += (el.numChildren - 1) * el.style.gap;
      } else {
        el.size.x += (el.numChildren - 1) * el.style.gap;
      }
    }

    float selfW = el.bound.get_width();
    float selfH = el.bound.get_height();

    if (el.type == ElementType::Container || el.type == ElementType::Button) {
      selfW = el.size.x + el.style.padding.left + el.style.padding.right;
      selfH = el.size.y + el.style.padding.top + el.style.padding.bottom;
    }
    el.bound.right = el.bound.left + selfW;
    el.bound.bottom = el.bound.top + selfH;

    if (el.parentIdx != NULLIDX) {
      auto& parent = m_elements[el.parentIdx];

      if (parent.style.direction == Direction::Vertical) {
        parent.size.y += selfH;
        parent.size.x += std::max(selfW, parent.size.x);
      } else {
        parent.size.y += std::max(selfH, parent.size.y);
        parent.size.x += selfW;
      }
    }
  }

  for (size_t i = 0; i < m_elements.size(); ++i) {
    auto& el = m_elements[i];
    float w = el.bound.get_width();
    float h = el.bound.get_height();

    if (el.parentIdx != NULLIDX) {
      auto& parent = m_elements[el.parentIdx];
      el.bound.left = parent.cursor.x;
      el.bound.top = parent.cursor.y;
      el.bound.right = el.bound.left + w;
      el.bound.bottom = el.bound.top + h;

      if (parent.style.direction == Direction::Vertical) {
        parent.cursor.y += h + parent.style.gap;
      } else {
        parent.cursor.x += w + parent.style.gap;
      }
    }

    el.cursor = glm::vec2(el.bound.left + el.style.padding.left,
                          el.bound.top + el.style.padding.top);
  }
}

uint32_t Context::push(Element e) {
  uint32_t idx = static_cast<uint32_t>(m_elements.size());
  e.parentIdx = m_parents.empty() ? NULLIDX : m_parents.back();
  e.size = glm::vec2(0.0f);
  e.cursor = glm::vec2(0.0f);
  e.numChildren = 0;

  if (e.parentIdx != NULLIDX) {
    m_elements[e.parentIdx].numChildren++;
  }

  m_elements.push_back(std::move(e));
  return idx;
}

void Context::pop() {
  m_parents.pop();
}

void Context::text(std::string_view label, float fontSize) {
  // TODO: I'm hard coding the font size right now.
  // In the future, it should have some responsive size wrt to
  // application resolution or sth.
  push(Element{
      .type = ElementType::Text,
      .bound =
          Bound2(0.0f, 0.0f, label.size() * 10.0f * fontSize, 24.0f * fontSize),
      .textData = label,
      .fontSize = fontSize,
  });
}

bool Context::button(std::string_view label, float fontSize) {
  uint32_t btnIdx = NULLIDX;
  container(
      Element{
          .type = ElementType::Button,
          .style = {.direction = Direction::Horizontal,
                    .padding = Bound2(8.0f, 4.0f, 8.0f, 4.0f)},
      },
      [&] {
        btnIdx = m_parents.back();
        text(label, fontSize);
      });

  const auto& b = m_elements[btnIdx].bound;
  bool hovered = (m_mousePos.x >= b.left && m_mousePos.x <= b.right &&
                  m_mousePos.y >= b.top && m_mousePos.y <= b.bottom);
  return hovered && m_isMouseDown;
}

void Context::render() {
  for (const auto& el : m_elements) {
    if (el.type == ElementType::Button) {
      glm::mat4 transform = glm::scale(
          glm::translate(glm::mat4(1.0f), {el.bound.left, el.bound.top, 0.0f}),
          {el.bound.get_width(), el.bound.get_height(), 1.0f});
      m_r2D.draw_rect(transform);
    } else if (el.type == ElementType::Text) {
      glm::mat4 transform = glm::scale(
          glm::translate(glm::mat4(1.0f), {el.bound.left, el.bound.top, 0.0f}),
          {el.fontSize, el.fontSize, 1.0f});
      m_r2D.draw_text(m_font, el.textData, transform);
    }
  }
}

}  // namespace ui
}  // namespace xev
