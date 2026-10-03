#include <xev/resource/buffer.h>

namespace xev {

void Buffer::write(const void* data,
                   const uint64_t size,
                   const uint64_t offset) {
  void* map_ = alloc_info.pMappedData;
  memcpy((char*)map_ + offset, data, size);
}

}  // namespace xev
