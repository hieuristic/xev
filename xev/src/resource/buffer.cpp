#include <xev/backend.h>
#include <xev/resource/buffer.h>

namespace xev {

void Buffer::reserve(VkDeviceSize size_, const Backend& backend) {
  size = size_;
  reserve(backend);
}

void Buffer::reserve(const Backend& backend) {
  backend.reserve_buffer(buffer, alloc, alloc_info, size, flags, usage);
}

void Buffer::release(const Backend& backend) {
  size = 0;
  backend.release_buffer(buffer, alloc);
}

void Buffer::upload(const void* src,
                    uint64_t offset,
                    uint64_t size_,
                    const Backend& backend) {
  backend.upload_buffer(src, alloc, offset, size_);
}

}  // namespace xev
