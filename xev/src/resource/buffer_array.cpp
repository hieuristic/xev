#include <xev/buffer_array.h>

namespace xev {

void BufferArray::set_size(uint64_t slotSize_) {
  slotSize = slotSize_;

  // round up multiple of MIN_ALIGNMENT
  uint64_t align = static_cast<uint64_t>(MIN_ALIGNMENT) - 1;
  stride = (slot_size + align) & ~align;
}

VkDeviceAddress BufferArray::get_addr(uint32_t idx) const {
  return addr + static_cast<VkDeviceAddress>(idx * stride);
}

void BufferArray::write(const uint32_t idx,
                        const void* data,
                        const uint64_t size,
                        const uint64_t offset) {
  void* map_ = alloc_info.pMappedData;
  memcpy((char*)map_ + idx * stride + offset, data, size);
}

}  // namespace xev
