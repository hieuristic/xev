#pragma once
#include <xev/resource/resource.h>

namespace xev {

class Sampler : Resource {
 public:
  uint64_t size_device() const override;
  bool is_reserved() const override;
  void reserve(const Backend& backend) override;
  void release(const Backend& backend) override;
  void upload(const Backend& backend);
};

}  // namespace xev
