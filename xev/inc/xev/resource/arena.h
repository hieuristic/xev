#pragma once
#include <xev/resource/buffer.h>

namespace xev {

struct Arena : Resource {
  Buffer buffer;
  uint64_t ptr{0};  // ptr into local memory space
  uint64_t avail_bytes() const { return buf.size - ptr; }
  uint64_t size_device() const override { return buf.size_device(); }
  bool on_device() const override { return buf.on_device(); }
  void write(const void* data, uint64_t size);
};

}  // namespace xev
