#pragma once
#include <xev/volk.h>

namespace xev {

class Backend;

class Resource {
 public:
  virtual uint64_t size_device() const = 0;
  virtual bool is_reserved() const = 0;
  virtual void reserve(const Backend& backend) = 0;
  virtual void release(const Backend& backend) = 0;
};

}  // namespace xev
