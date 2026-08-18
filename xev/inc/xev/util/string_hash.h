#pragma once
#include <functional>
#include <string>
#include <string_view>

namespace xev {

struct StringHash {
  using is_transparent = void;

  size_t operator()(std::string_view sv) const {
    return std::hash<std::string_view>{}(sv);
  }
};

}  // namespace xev
