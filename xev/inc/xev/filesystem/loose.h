#pragma once
#include <xev/filesystem/source.h>

namespace xev {

struct LooseSource : public FileSource {
  LooseSource(std::string_view path_);
  std::unorderred_map<std::string, > locator;
  void read() override;
  void index() override;
}

}  // namespace xev
