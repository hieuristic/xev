#pragma once
#include <xev/filesystem/mount.h>
#include <filesystem>
#include <span>
#include <unordered_map>

namespace xev {

enum ARXEntryFlag {};

#pragma pack(push, 1)
struct ARXHeader {
  char magic[4]{'A', 'R', 'X', 'I'};
  uint32_t version{1};
  uint32_t numEntrys{0};
  uint64_t dirOffset{0};
};
struct ARXEntry {
  uint64_t pathHash{0};
  uint64_t offset{0};
  uint64_t sizeCompress{0};
  uint64_t sizeOriginal{0};
  uint32_t flags{0};
};
#pragma pack(pop)

struct ArxivMount : public Mount {
  explicit ArxivMount(std::filesystem::path arxPath_);
  ~ArxivMount() override;

  void index() override;

  static bool pack(std::filesystem::path& targetDir,
                   std::filesystem::path& writeFile);

  bool parse_header();

  std::vector<uint8_t> read(std::string_view path,
                            uint64_t offset,
                            uint64_t count) override;

  std::span<const uint8_t> read_view(std::string_view path,
                                     uint64_t offset,
                                     uint64_t count) const;

  std::filesystem::path arxPath;

 private:
  uint64_t m_fileSize{0};
  const uint8_t* m_mmapData{nullptr};
  std::unordered_map<uint64_t, const ARXEntry*> m_entries;
};

}  // namespace xev
