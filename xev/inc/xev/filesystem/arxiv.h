#pragma once
#include <xev/filesystem/mount.h>
#include <filesystem>

namespace xev {

struct ArxivLocation {
  uint64_t offset;
  uint64_t sizeCompressed;
  uint64_t sizeUncompressed;
};

struct ArxivMount : public Mount {
  explicit ArxivMount(std::filesystem::path arxPath_);
  void index() override;
  std::vector<uint8_t> read(std::string_view path,
                            uint64_t offset,
                            uint64_t count) override;

  std::filesystem::path arxPath;

 private:
  // mutable
};

}  // namespace xev
