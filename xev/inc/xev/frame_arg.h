#pragma once
#include <xev/volk.h>
#include <array>

namespace xev {

struct FrameArg {
  bool copy_to_swapchain;
  std::array<float, 4> clear_color;
  VkFilter filter;
};

}  // namespace xev
