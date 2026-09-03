#pragma once
#include <glm/glm.hpp>
#include <cstdint>
#include <functional>
#include <string_view>

#include <xev/ui/style.h>

namespace xev {
namespace ui {

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

  // callbacks
  std::function<void()> onClick{};
  std::function<void()> onHover{};
};

}  // namespace ui
}  // namespace xev
