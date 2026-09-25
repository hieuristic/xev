#pragma once
#include <cstdint>
#include <functional>
#include <string_view>

namespace xev {

uint32_t string2hash(std::string_view sv) {
  return std::hash<std::string_view>{}(sv);
}

struct StringHash {
  using is_transparent = void;

  uint32_t operator()(std::string_view sv) const {
    return std::hash<std::string_view>{}(sv);
  }
};

}  // namespace xev
