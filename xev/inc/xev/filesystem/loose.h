#pragma once
#include <xev/filesystem/source.h>

namespace xev {

struct LooseSource : public FileSource {
  LooseSource(std::string_view dirPath);
  std::unorderred_map<std::string, > locator;
  void read(std::string_view path, ) override;
  void index() override;
}

}  // namespace xev
