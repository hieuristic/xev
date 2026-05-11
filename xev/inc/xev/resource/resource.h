#pragma once
#include <xev/volk.h>

namespace xev {

class Backend;

class Resource {
 public:
  virtual uint64_t size_device() const = 0;
  virtual uint64_t size_host() const = 0;
  virtual bool is_loaded() const = 0;
  virtual void load(const Backend& backend) = 0;
  virtual void unload(const Backend& backend) = 0;
};

}  // namespace xev
