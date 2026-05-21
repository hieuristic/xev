#include <xev/backend.h>
#include <xev/common.h>
#include <xev/resource/texture.h>

namespace xev {

uint64_t size_device() const {
  return image.size_device() + sampler.size_device();
}

bool is_reserved() const {
  return image.is_reserved() + sampler.is_reserved();
}

void Texture::reserve(uint32_t width_,
                      uint32_t height_,
                      const Backend& backend) {
  image.reserve(width_, height_, backend);
}

void Texture::reserve(const Backend& backend) {
  image.reserve(backend);
}

void Texture::release(const Backend& backend) {
  image.release(backend);
}

void Texture::set_layout(const VkImageLayout& layout_) {
  image.set_layout(layout_);
}

void Texture::update_layout(const VkCommandBuffer& cmd,
                            const VkImageLayout& new_layout) {
  image.update_layout(cmd, new_layout);
}

void Texture::copy(const VkCommandBuffer& cmd,
                   const Texture& src,
                   const VkFilter& filter) {
  image.copy(cmd, src, filter);
}

}  // namespace xev
