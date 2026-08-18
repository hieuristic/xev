#pragma once
#include <xev/filesystem/const.h>
#include <xev/filesystem/mount.h>
#include <filesystem>

namespace xev {

struct LooseMount : public Mount {
  explicit LooseMount(std::filesystem::path rootPath_);

  void index() override;
  std::vector<uint8_t> read(std::string_view path,
                            uint64_t offset = 0,
                            uint64_t count = fs::EOF_COUNT) override;

  std::filesystem::path rootPath;
};

}  // namespace xev
