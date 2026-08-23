#pragma once
#include <xev/geometry/bound.h>
#include <memory>
#include <vector>

namespace xev {
namespace ui {

struct Node {
  virtual ~Node() = default;

  // No copy, move
  Node(const Node&) = delete;
  Node& operator=(const Node&) = delete;
  Node(Node&&) = default;
  Node& operator=(Node&&) = default;

  template <typename T>
    requires std::derived_from<T, Node>
  T* add_child(std::unique_ptr<T> child) {
    if (!child) {
      return nullptr;
    }

    child->parent = this;
    T* ptr = child.get();
    children.push_back(std::move(child));
    dirty = true;
    return ptr;
  }

  void compute();
  void measure();
  void arrange();

  virtual void draw(Renderer2D& r2D) = 0;
  virtual Bound2 measure_self();  // for objects like text

  Node* parent{nullptr};
  std::vector<std::unique_ptr<Node>> m_children;

  Style style{};
  Bound2 outer_bound{};
  Bound2 inner_bound{};
  bool dirty{true};
  bool visible{true};
}

}  // namespace ui
}  // namespace xev

