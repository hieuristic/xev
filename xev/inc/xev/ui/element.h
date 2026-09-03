#pragma once
#include <cstdint>
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
};

}  // namespace ui
}  // namespace xev
