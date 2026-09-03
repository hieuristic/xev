#include <xev/logger.h>
#include <xev/ui/layout.h>
#include <algorithm>
#include <glm/glm.hpp>

namespace xev {
namespace ui {

void Context::solve() {
  if (m_elements.empty()) return;

  // 1. aggregate children size to parent
  for (size_t i = m_elements.size(); i--;) {
    auto& el = m_elements[i];
    if (el.numChildren > 0) {
      if (el.style.direction == Direction::Vertical) {
        el.size.y += (el.numChildren - 1) * el.style.gap;
      } else {
        el.size.x += (el.numChildren - 1) * el.style.gap;
      }
    }

    float selfW = el.bound.get_width();
    float selfH = el.bound.get_height();

    if (el.type != ElementType::Text) {
      if (el.style.sizing.type == SizingType::Fixed) {
        selfW = el.style.sizing.value;
        selfH = el.style.sizing.value;
      } else {
        // SizingType::Fit (Default): Wrap children content + padding
        selfW = el.size.x + el.style.padding.left + el.style.padding.right;
        selfH = el.size.y + el.style.padding.top + el.style.padding.bottom;
      }
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

      // SizingType::Grow, distribute equally among children
      if (el.style.sizing.type == SizingType::Grow &&
          parent.numGrowChildren > 0) {
        if (parent.style.direction == Direction::Vertical) {
          float innerH =
              parent.bound.get_height() -
              (parent.style.padding.top + parent.style.padding.bottom);
          float remaining = innerH - parent.size.y;
          if (remaining > 0.0f) h += remaining / parent.numGrowChildren;
        } else {
          float innerW =
              parent.bound.get_width() -
              (parent.style.padding.left + parent.style.padding.right);
          float remaining = innerW - parent.size.x;
          if (remaining > 0.0f) w += remaining / parent.numGrowChildren;
        }
      }

      // SizingType::Percent: Calculate proportional size from parent
      if (el.style.sizing.type == SizingType::Percent) {
        float perCent = el.style.sizing.value * 0.01f;
        if (parent.style.direction == Direction::Vertical) {
          h = parent.bound.get_height() * perCent;
        } else {
          w = parent.bound.get_width() * perCent;
        }
      }

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

uint32_t Context::push(Element&& e) {
  uint32_t idx = static_cast<uint32_t>(m_elements.size());
  e.parentIdx = m_parents.empty() ? NULLIDX : m_parents.back();
  e.size = glm::vec2(0.0f);
  e.cursor = glm::vec2(0.0f);
  e.numChildren = 0;

  if (e.parentIdx != NULLIDX) {
    m_elements[e.parentIdx].numChildren++;
    if (e.style.sizing.type == SizingType::Grow)
      m_elements[e.parentIdx].numGrowChildren++;
  }

  m_elements.push_back(std::move(e));
  // m_parents.push_back(idx);
  return idx;
}

void Context::text(std::string_view label, float fontSize, Style&& style) {
  // TODO: I'm hard coding the font size right now.
  // In the future, it should have some responsive size wrt to
  // application resolution or sth.
  //
  float pixelScale = 28.0f * fontSize;
  glm::vec2 selfSize = m_font.measure(label) * pixelScale;

  push(Element{
      .type = ElementType::Text,
      .bound = Bound2(0.0f, 0.0f, selfSize.x, selfSize.y),
      .textData = label,
      .fontSize = fontSize,
  });
}

bool Context::button(std::string_view label,
                     float fontSize,
                     glm::vec3& color,
                     Style&& style) {
  uint32_t btnIdx = push(Element{
      .type = ElementType::Button,
      .style = style,
      .color = color,
  });

  m_parents.push_back(btnIdx);
  text(label, fontSize);
  m_parents.pop_back();

  if (btnIdx >= m_elements.size()) return false;

  const auto& b = m_elements[btnIdx].bound;
  XEV_INFO("{} {} {}", m_mousePos.x, b.left, b.right);
  XEV_INFO("{} {} {}", m_mousePos.y, b.top, b.bottom);
  bool hovered = (m_mousePos.x >= b.left && m_mousePos.x <= b.right &&
                  m_mousePos.y >= b.top && m_mousePos.y <= b.bottom);
  if (hovered)
    XEV_INFO("Hovered");
  return hovered && m_isMouseDown;
}

void Context::render() {
  if (m_elements.empty()) return;

  float screenW = m_elements[0].bound.get_width();
  float screenH = m_elements[0].bound.get_height();

  for (const auto& el : m_elements) {
    if (el.type == ElementType::Button) {
      float w = el.bound.get_width();
      float h = el.bound.get_height();
      float scaleX = w / screenW;
      float scaleY = h / screenH;
      float posX = ((el.bound.left + w * 0.5f) / screenW) * 2.0f - 1.0f;
      float posY = ((el.bound.top + h * 0.5f) / screenH) * 2.0f - 1.0f;

      glm::mat3 transform =
          glm::mat3(glm::vec3(scaleX, 0.0f, 0.0f),
                    glm::vec3(0.0f, scaleY, 0.0f), glm::vec3(posX, posY, 1.0f));
      m_r2D.draw_rect(transform, el.color);
    } else if (el.type == ElementType::Text) {
      float pixelScale = 28.0f * el.fontSize;
      float scaleX = pixelScale * 2 / screenW;
      float scaleY = pixelScale * 2 / screenH;
      float posX = (el.bound.left / screenW) * 2.0f - 1.0f;
      float posY = (el.bound.top / screenH) * 2.0f - 1.0f;

      glm::mat3 transform =
          glm::mat3(glm::vec3(scaleX, 0.0f, 0.0f),
                    glm::vec3(0.0f, scaleY, 0.0f), glm::vec3(posX, posY, 1.0f));

      m_r2D.draw_text(m_font, el.textData, transform);
    }
  }
}

void Context::print_tree_layout() const {
  XEV_INFO("========== [UI Layout Tree] ==========");
  for (size_t i = 0; i < m_elements.size(); ++i) {
    const auto& el = m_elements[i];

    // Compute tree depth for visual indentation
    uint32_t depth = 0;
    uint32_t p = el.parentIdx;
    while (p != NULLIDX) {
      depth++;
      p = m_elements[p].parentIdx;
    }
    std::string indent(depth * 2, ' ');

    std::string typeStr = (el.type == ElementType::Container) ? "Container"
                          : (el.type == ElementType::Button)  ? "Button"
                                                              : "Text";
    float w = el.bound.get_width();
    float h = el.bound.get_height();

    if (el.type == ElementType::Text) {
      XEV_INFO(
          "{}[{}] \"{}\" Bounds: [L:{:.1f}, T:{:.1f}, R:{:.1f}, B:{:.1f}] "
          "Size: {:.1f}x{:.1f}",
          indent, typeStr, el.textData, el.bound.left, el.bound.top,
          el.bound.right, el.bound.bottom, w, h);
    } else {
      XEV_INFO(
          "{}[{}] Bounds: [L:{:.1f}, T:{:.1f}, R:{:.1f}, B:{:.1f}] Size: "
          "{:.1f}x{:.1f} Gap: {:.1f}",
          indent, typeStr, el.bound.left, el.bound.top, el.bound.right,
          el.bound.bottom, w, h, el.style.gap);
    }
  }
  XEV_INFO("======================================");
}

}  // namespace ui
}  // namespace xev
