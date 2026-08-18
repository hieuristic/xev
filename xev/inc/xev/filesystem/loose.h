#pragma once
#include <xev/filesystem/mount.h>

namespace xev {

struct LooseSource : public Mount {
  LooseSource(std::string_view dirPath);
  std::unorderred_map<std::string, > locator;
  void read(std::string_view path, ) override;
  void index() override;
}

}  // namespace xev
