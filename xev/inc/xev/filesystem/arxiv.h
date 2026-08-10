#pragma once
#include <xev/filesystem/source.h>

namespace xev {

struct ArxivLocation {
  uint64_t offset;
  uint64_t sizeCompressed;
  uint64_t sizeUncompressed;
};

struct ArxivSource : public FileSource {
  ArxivSource(std::string_view path_);

  ARX arx;
  std::unordered_map<std::string> directive;

  void index() override;
private:
  mutable
};

}  // namespace xev
