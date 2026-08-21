#include <sys/mman.h>
#include <unistd.h>
#include <xev/filesystem/arxiv.h>
#include <xev/filesystem/const.h>
#include <xev/util/string_hash.h>
#include <algorithm>
#include <fstream>

namespace xev {

ArxivMount::ArxivMount(std::filesystem::path arxPath_)
    : arxPath(std::move(arxPath_)) {}

ArxivMount::~ArxivMount() {
  if (m_mmapData && m_mmapData != MAP_FAILED) {
    munmap(const_cast<uint8_t*>(m_mmapData), m_fileSize);
  }
}

void ArxivMount::index() {}

bool ArxivMount::parse_header() {
  return false;
}

// TODO: current not compressing.
bool ArxivMount::pack(std::filesystem::path& targetDir,
                      std::filesystem::path& writeFile) {
  if (!std::filesystem::exists(targetDir) ||
      !std::filesystem::is_directory(targetDir)) {
    return false;
  }

  if (writeFile.has_parent_path()) {
    std::filesystem::create_directories(writeFile.parent_path());
  }

  std::ofstream out(writeFile, std::ios::binary);
  if (!out.is_open())
    return false;

  ARXHeader header{};
  out.write(reinterpret_cast<const char*>(&header), sizeof(header));

  std::vector<ARXEntry> entries;
  std::vector<char> buffer(64 * 1024);  // 64kB

  for (const auto& dirEntry :
       std::filesystem::recursive_directory_iterator(targetDir)) {
    if (!dirEntry.is_regular_file())
      continue;

    // avoid packing itself
    if (std::filesystem::equivalent(dirEntry.path(), writeFile)) {
      continue;
    }

    std::string filename = dirEntry.path().filename().string();
    if (!filename.empty() && filename[0] == '.')
      continue;

    std::filesystem::path relPath =
        std::filesystem::relative(dirEntry.path(), targetDir);
    std::string relPathStr = relPath.generic_string();

    std::ifstream in(dirEntry.path(), std::ios::binary | std::ios::ate);
    if (!in.is_open())
      return false;

    uint64_t fileSize = static_cast<uint64_t>(in.tellg());
    in.seekg(0, std::ios::beg);

    // align by 8 bytes
    std::streampos currPos = out.tellp();
    uint64_t pad = (8 - (static_cast<uint64_t>(currPos) % 8)) % 8;
    if (pad > 0) {
      char toPad[8]{0};
      out.write(toPad, pad);
    }

    uint64_t offset = static_cast<uint64_t>(out.tellp());

    // write file
    while (in) {
      in.read(buffer.data(), buffer.size());
      auto hasRead = in.gcount();
      if (hasRead > 0) {
        out.write(buffer.data(), hasRead);
      }
    }

    ARXEntry entry{
        .pathHash = StringHash{}(relPathStr),
        .offset = offset,
        .sizeCompress = fileSize,  // TODO: compression
        .sizeOriginal = fileSize,
        .flags = 0,
    };

    entries.emplace_back(entry);
  }

  uint64_t entryOffset = static_cast<uint64_t>(out.tellp());
  out.write(reinterpret_cast<const char*>(entries.data()),
            entries.size() * sizeof(ARXEntry));

  header.numEntrys = static_cast<uint32_t>(entries.size());
  header.dirOffset = entryOffset;
  out.seekp(0, std::ios::beg);
  out.write(reinterpret_cast<const char*>(&header), sizeof(header));
  return true;
}

std::vector<uint8_t> ArxivMount::read(std::string_view path,
                                      uint64_t offset,
                                      uint64_t count) {
  auto view = read_view(path, offset, count);
  return {view.begin(), view.end()};
}

std::span<const uint8_t> ArxivMount::read_view(std::string_view path,
                                               uint64_t offset,
                                               uint64_t count) const {
  if (!m_mmapData)
    return {};

  uint64_t hash = StringHash{}(path);
  auto it = m_entries.find(hash);
  if (it == m_entries.end()) {
    return {};
  }

  const ARXEntry* entry = it->second;
  if (offset >= entry->sizeOriginal) {
    return {};
  }

  uint64_t toRead =
      (count == fs::EOF_COUNT) ? (entry->sizeOriginal - offset) : count;
  toRead = std::min(toRead, entry->sizeOriginal - offset);

  return {m_mmapData + entry->offset + offset, toRead};
}

}  // namespace xev
