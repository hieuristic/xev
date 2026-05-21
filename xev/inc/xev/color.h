#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace xev {

class Color3 {
 public:
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;

  Color3(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
  Color3() : r(0), g(0), b(0) {}

  Color3 linear2srgb() const {
    static const auto srgb_lut = []() {
      std::array<uint8_t, 256> table;
      for (int i = 0; i < 256; ++i) {
        // 1. Normalize to 0.0 - 1.0
        float lin = static_cast<float>(i) / 255.0f;
        float s;

        // 2. Apply piecewise sRGB conversion
        if (lin <= 0.0031308f) {
          s = lin * 12.92f;
        } else {
          s = 1.055f * std::pow(lin, 1.0f / 2.4f) - 0.055f;
        }

        // 3. Scale back to 255 and round correctly (+0.5f)
        table[i] =
            static_cast<uint8_t>(std::clamp(s * 255.0f + 0.5f, 0.0f, 255.0f));
      }
      return table;
    }();

    return Color3(srgb_lut[r], srgb_lut[g], srgb_lut[b]);
  }
};

class Color4 {
 public:
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;

  Color4(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}
  Color4(const std::vector<double>& color) {
    r = static_cast<uint8_t>(color[0] * 255.0);
    g = static_cast<uint8_t>(color[1] * 255.0);
    b = static_cast<uint8_t>(color[2] * 255.0);
    a = static_cast<uint8_t>(color[3] * 255.0);
  }
  Color4() : r(255), g(255), b(255), a(255) {}

  Color4 linear2srgb() const {
    static const auto srgb_lut = []() {
      std::array<uint8_t, 256> table;
      for (int i = 0; i < 256; ++i) {
        // 1. Normalize to 0.0 - 1.0
        float lin = static_cast<float>(i) / 255.0f;
        float s;

        // 2. Apply piecewise sRGB conversion
        if (lin <= 0.0031308f) {
          s = lin * 12.92f;
        } else {
          s = 1.055f * std::pow(lin, 1.0f / 2.4f) - 0.055f;
        }

        // 3. Scale back to 255 and round correctly (+0.5f)
        table[i] =
            static_cast<uint8_t>(std::clamp(s * 255.0f + 0.5f, 0.0f, 255.0f));
      }
      return table;
    }();

    return Color4(srgb_lut[r], srgb_lut[g], srgb_lut[b], a);
  }
};

class Color4HighP {
 public:
  double r;
  double g;
  double b;
  double a;

  Color4HighP(double r, double g, double b, double a)
      : r(r), g(g), b(b), a(a) {}
  Color4HighP() : r(1.0), g(1.0), b(1.0), a(1.0) {}
  Color4HighP linear2srgb() const {
    static auto convert = [](double x) -> double {
      if (x <= 0.0031308)
        return 12.92 * x;
      else
        return 1.055 * std::pow(x, (1.0 / 2.4)) - 0.055;
    };

    return Color4HighP(convert(r), convert(g), convert(b), a);
  }
};
}  // namespace xev
//
