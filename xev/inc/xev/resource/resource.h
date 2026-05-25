#pragma once
#include <xev/volk.h>

namespace xev {

class Backend;

class Resource {
  virtual uint64_t size_device() const = 0;
  virtual bool on_device() const = 0;
};

}  // namespace xev
