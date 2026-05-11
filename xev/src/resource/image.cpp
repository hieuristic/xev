#include <xev/backend.h>
#include <xev/resource/image.h>

namespace xev {

void Image::load(uint32_t width_, uint32_t height_, const Backend& backend) {
  width = width_;
  height = height_;
  load(backend);
}
void Image::load(const Backend& backend) {
  backend.load_image(image, view, alloc, alloc_info, width, height, format,
                      flags);
}
void Image::unload(const Backend& backend) {
  backend.unload_image(image, alloc, view);
}
}  // namespace xev
