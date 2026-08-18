#pragma once
#include <xev/resource/resource.h>

namespace xev {

enum SamplerType {
  SAMPLER_LINEAR
};

constexpr float MAX_SAMPLER_ANISOTROPY = 1.0;

struct Sampler : public Resource {
  uint64_t size_device() const override { return 0; }
  bool on_device() const override { return sampler != VK_NULL_HANDLE; }

  Sampler(SamplerType type_) : type(type_) {}
  SamplerType type{SAMPLER_LINEAR};
  VkSampler sampler{VK_NULL_HANDLE};
};

}  // namespace xev
