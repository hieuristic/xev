#pragma once
#include <xev/geometry/bound.h>
#include <cstdint>

/* Thank you https://github.com/nicbarker/clay
 * for some inspiration :) */

namespace xev {
namespace ui {

enum struct Direction : uint8_t {
  Vertical,
  Horizontal,
};

enum struct AlignX {
  Left,
  Center,
  Right,
};

enum struct AlignY {
  Top,
  Center,
  Bottom,
};

enum struct SizingType {
  Fit,
  Grow,
  Fixed,
  Percent,
};

struct Sizing {
  SizingType type{SizingType::Fit};
  float value{0.0};
};

struct Style {
  Direction direction;
  AlignX alignX{AlignX::Left};
  AlignY alignY{AlignY::Top};
  Sizing sizing{};
  Bound2 padding{};
  float gap{0.0f};
};

}  // namespace ui
}  // namespace xev
