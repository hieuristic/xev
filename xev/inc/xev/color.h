#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace xev {

template <typename T>
concept ColorType = std::same_as<T, uint8_t> || std::same_as<T, float> ||
                    std::same_as<T, double>;

template <size_t N>
concept ColorDim = (N == 3 || N == 4);

// N defaults to 4 if not specified
template <ColorType T, size_t N = 4>
  requires ColorDim<N>
class Color {
 public:
  T r, g, b;

  struct Empty {};
  [[no_unique_address]] std::conditional_t<N == 4, T, Empty> a;

  Color(T r, T g, T b)
    requires(N == 3)
      : r(r), g(g), b(b) {}

  Color(T r, T g, T b, T a)
    requires(N == 4)
      : r(r), g(g), b(b), a(a) {}

  template <std::floating_point F>
  explicit Color(const std::vector<F>& color) {
    if (color.size() < N) {
      throw std::out_of_range("Vector does not contain enough channels.");
    }

    auto scale = [](F val) -> T {
      if constexpr (std::same_as<T, uint8_t>) {
        return static_cast<uint8_t>(
            std::clamp(val * F(255.0) + F(0.5), F(0.0), F(255.0)));
      } else {
        return static_cast<T>(val);
      }
    };

    r = scale(color[0]);
    g = scale(color[1]);
    b = scale(color[2]);
    if constexpr (N == 4) {
      a = scale(color[3]);
    }
  }

  Color() {
    if constexpr (std::same_as<T, uint8_t>) {
      r = g = b = 255;
      if constexpr (N == 4)
        a = 255;
    } else {
      r = g = b = static_cast<T>(1.0);
      if constexpr (N == 4)
        a = static_cast<T>(1.0);
    }
  }

  Color linear2srgb() const {
    if constexpr (std::same_as<T, uint8_t>) {
      static const auto srgb_lut = []() {
        std::array<uint8_t, 256> table;
        for (int i = 0; i < 256; ++i) {
          float lin = static_cast<float>(i) / 255.0f;
          float s = (lin <= 0.0031308f)
                        ? (lin * 12.92f)
                        : (1.055f * std::pow(lin, 1.0f / 2.4f) - 0.055f);
          table[i] =
              static_cast<uint8_t>(std::clamp(s * 255.0f + 0.5f, 0.0f, 255.0f));
        }
        return table;
      }();

      if constexpr (N == 4)
        return Color(srgb_lut[r], srgb_lut[g], srgb_lut[b], a);
      else
        return Color(srgb_lut[r], srgb_lut[g], srgb_lut[b]);

    } else {
      auto convert = [](T x) -> T {
        if (x <= static_cast<T>(0.0031308))
          return static_cast<T>(12.92) * x;
        else
          return static_cast<T>(1.055) *
                     std::pow(x, static_cast<T>(1.0 / 2.4)) -
                 static_cast<T>(0.055);
      };

      if constexpr (N == 4)
        return Color(convert(r), convert(g), convert(b), a);
      else
        return Color(convert(r), convert(g), convert(b));
    }
  }
};

template <typename T = float>
using Color3 = Color<T, 3>;
template <typename T = float>
using Color4 = Color<T, 4>;

}  // namespace xev
