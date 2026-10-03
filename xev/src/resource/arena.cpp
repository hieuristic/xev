#include <xev/resource/arena.h>

namespace xev {

void Arena::write(const void* data, uint64_t size) {
  XEV_ASSERT(ptr + size < buf.size,
             "[arena] not enough space in arena for writing");
  buffer.write(mem, size, ptr);
  ptr += size;
}

void Arena::set_ptr(uint64_t new_ptr) {
  XEV_ASSERT(new_ptr < size && new ptr >= 0,
             "[arena] trying to set invalid ptr");
  ptr = new_ptr;
}

void Arena::clear() {
  ptr = 0;
}

}  // namespace xev
