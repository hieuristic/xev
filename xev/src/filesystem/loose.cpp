#include <xev/filesystem/loose.h>
#include <xev/logger.h>
#include <fstream>

namespace xev {

LooseMount::LooseMount(std::filesystem::path rootPath_)
    : rootPath(std::move(rootPath_)) {
  index();
}

void LooseMount::index() {
  if (!std::filesystem::exists(rootPath) ||
      !std::filesystem::is_directory(rootPath)) {
    XEV_WARN("[Loose] Failed to mount {}: doesn't exist or is not a folder.",
             rootPath.generic_string());
    return;
  }

  directive.clear();

  for (const auto& entry : std::filesystem::recursive_directory_iterator(
           rootPath,
           std::filesystem::directory_options::follow_directory_symlink)) {
      if (entry.is_regular_file()) {
        std::string relPath =
            std::filesystem::relative(entry.path(), rootPath).generic_string();
        directive[relPath] =
            0;  // directive value doesn't matter in loose files.
      }
    }
}

std::vector<uint8_t> LooseMount::read(std::string_view path,
                                      uint64_t offset,
                                      uint64_t count) {
  std::filesystem::path fullPath = rootPath / path;
  std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    XEV_ERROR("[Loose] failed to open file {}", fullPath.generic_string());
    return {};
  }

  uint64_t fileSize = static_cast<uint64_t>(file.tellg());
  if (offset >= fileSize) {
    return {};
  }

  uint64_t toRead = (count == fs::EOF_COUNT) ? (fileSize - offset) : count;
  toRead = std::min(toRead, fileSize - offset);

  std::vector<uint8_t> buffer(toRead);
  file.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
  file.read(reinterpret_cast<char*>(buffer.data()), toRead);

  return buffer;
}

}  // namespace xev
