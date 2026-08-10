#include <xev/filesystem/loose.h>

namespace xev {

std::vector<uint8_t> LooseSource::read(std::string_view path,
                                       uint64_t offset,
                                       uint64_t count) {
  FILE* f = fopen(full_path.c_str(), "rb");
  if (!f) return {};

  fseek(f, 0, SEEK_END);
  uint64_t totalSize = ftell(f);

  if (offset >= totalSize) {
    fclose(f);
    return {};
  }

}

}  // namespace xev
