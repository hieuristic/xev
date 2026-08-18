#pragma once
#include <xev/volk.h>

namespace xev {

struct Backend;

struct Resource {
  virtual uint64_t size_device() const = 0;
  virtual bool on_device() const = 0;
};

}  // namespace xev
