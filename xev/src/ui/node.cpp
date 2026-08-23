#include <xev/ui/node.h>

namespace xev {
namespace ui {

void Node::compute() {
  if (!visible)
    return;
  measure();
  arrange();
  dirty = false;
}

void Node::measure() {
  for (auto& child : children) {
    if (child->visible)
      child->measure();
  }

  Bound2 selfBound = measure_self();
}

void Node::arrange() {
  ;
}

}  // namespace ui
}  // namespace xev
