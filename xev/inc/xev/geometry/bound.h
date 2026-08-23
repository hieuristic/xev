#pragma once

namespace xev {

struct Bound2 {
  float top{0.0};
  float left{0.0};
  float right{0.0};
  float bottom{0.0};
};

struct Bound3 {
  float top{0.0};
  float left{0.0};
  float front{0.0};
  float right{0.0};
  float bottom{0.0};
  float back{0.0};
};

}
