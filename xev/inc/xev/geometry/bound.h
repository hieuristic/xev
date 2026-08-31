#pragma once
#include <cmath>

namespace xev {

struct Bound2 {
  Bound2(float val) {
    top = val;
    left = val;
    right = val;
    bottom = val;
  };

  Bound2(float valW, float valH) {
    top = valH;
    left = valW;
    right = valW;
    bottom = valH;
  }

  Bound2(float t, float l, float r, float b) {
    top = t;
    left = l;
    right = r;
    bottom = b;
  }

  float top{0.0};
  float left{0.0};
  float right{0.0};
  float bottom{0.0};
  float get_width() const { return std::abs(right - left); }
  float get_height() const { return std::abs(bottom - top); }
};

struct Bound3 {
  float top{0.0};
  float left{0.0};
  float front{0.0};
  float right{0.0};
  float bottom{0.0};
  float back{0.0};
};

}  // namespace xev
