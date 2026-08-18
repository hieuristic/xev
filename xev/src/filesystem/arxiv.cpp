#include <xev/filesystem/arxiv.h>

namespace xev {

ArxivMount::ArxivMount() {}
ArxivMount::index() {}
std::vector<uint8_t> ArxivMount::read(std::string_view path,
                                      uint64_t offset,
                                      uint64_t count) {
  return {};
}

}  // namespace xev
