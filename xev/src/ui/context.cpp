#include <xev/ui/node.h>
#include <algorithm>

namespace xev {
namespace ui {

void Context::solve_sizing(uint32_t idx) {
  auto& el = m_elements[idx];
  for (uint32_t i = el.firstChildIdx; i < el.lastChildIdx; ++i) {
    solve_sizing(i);
  }
}

void Context::push(Element e) {
  uint32_t idx = static_cast<uint32_t>(m_elements.size());
  uint32_t parentIdx = m_parents.empty() ? NULLIDX : m_parents.back();
  e.parentIdx = parentIdx;
  e.firstChildIdx = NULLIDX;
  e.lastChildIdx = NULLIDX;
  e.siblingIdx = NULLIDX;
  m_elements.push_back(e);

  if (parentIdx != NULLIDX) {
    auto& parent = m_elements[parentIdx];
    if (parent.firstChildIdx == NULLIDX) {
      parent.firstChildIdx = idx;
    }
    else {
      m_elements[parent.lastChildIdx].siblingIdx = idx;
    }
    parent.lastChildIdx = idx;
  }

  return idx;
}

void Context::text() {}

void Context::button() {}

}  // namespace ui
}  // namespace xev
