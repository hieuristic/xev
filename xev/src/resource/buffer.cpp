#include <xev/backend.h>
#include <xev/resource/buffer.h>

namespace xev {

void Buffer::load(VkDeviceSize size_, const Backend& backend) {
  size = size_;
  load(backend);
}

void Buffer::load(const Backend& backend) {
  backend.load_buffer(buffer, alloc, alloc_info, size, flags, usage);
}

void Buffer::unload(const Backend& backend) {
  size = 0;
  backend.unload_buffer(buffer, alloc);
}

void Buffer::copy(const void* src,
                  uint64_t offset,
                  uint64_t size_,
                  const Backend& backend) {
  backend.copy_buffer(src, alloc, offset, size_);
}

}  // namespace xev
