#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <xev/filesystem/arxiv.h>
#include <xev/filesystem/const.h>
#include <xev/logger.h>
#include <xev/util/string_hash.h>
#include <algorithm>
#include <fstream>

namespace xev {

ArxivMount::ArxivMount(std::filesystem::path arxPath_)
    : arxPath(std::move(arxPath_)) {
  index();
  XEV_INFO("MOUNTED ARXIV");
}

ArxivMount::~ArxivMount() {
  if (m_mmap && m_mmap != MAP_FAILED) {
    munmap(const_cast<uint8_t*>(m_mmap), m_fileSize);
  }
}

ArxivMount::ArxivMount(ArxivMount&& other) noexcept
    : Mount(std::move(other)),
      arxPath(std::move(other.arxPath)),
      m_fileSize(other.m_fileSize),
      m_mmap(other.m_mmap),
      m_entries(std::move(other.m_entries)),
      m_header(other.m_header) {
  other.m_mmap = nullptr;
  other.m_fileSize = 0;
}

ArxivMount& ArxivMount::operator=(ArxivMount&& other) noexcept {
  if (this != &other) {
    Mount::operator=(std::move(other));
    arxPath = std::move(other.arxPath);
    m_fileSize = other.m_fileSize;
    m_mmap = other.m_mmap;
    m_entries = std::move(other.m_entries);
    m_header = other.m_header;
    other.m_mmap = nullptr;
    other.m_fileSize = 0;
  }
  return *this;
}

void ArxivMount::index() {
  if (!std::filesystem::exists(arxPath)) {
    XEV_ERROR("[ArxivMount] Input arxPath doesn't exist : {}",
              arxPath.string());
    return;
  }

  std::ifstream in(arxPath, std::ios::binary);
  if (!in.is_open())
    return;

  in.read(reinterpret_cast<char*>(&m_header), sizeof(m_header));
  if (std::string_view(m_header.magic, 4) != std::string_view("ARXI", 4)) {
    XEV_ERROR("[ArxivMount] Trying to read non ARX file : {}",
              arxPath.string());
    return;
  }

  // read entries and translation table
  std::vector<ARXEntry> entries(m_header.numEntry);
  in.seekg(m_header.offsetEntry);
  in.read(reinterpret_cast<char*>(entries.data()),
          sizeof(ARXEntry) * m_header.numEntry);
  std::vector<char> translationString(m_header.sizeTranslation);
  in.read(translationString.data(), translationString.size());

  m_entries.clear();
  directive.clear();

  const char* nextStrPtr = translationString.data();
  for (auto& entry : entries) {
    std::string entryPath(nextStrPtr);
    nextStrPtr += entryPath.size() + 1;

    m_entries[StringHash{}(entryPath)] = entry;
    directive[entryPath] = 0;
  }

  int fd = open(arxPath.c_str(), O_RDONLY);
  if (fd != -1) {
    struct stat st{};
    if (fstat(fd, &st) == 0) {
      m_fileSize = static_cast<uint64_t>(st.st_size);
      m_mmap = static_cast<const uint8_t*>(
          mmap(nullptr, m_fileSize, PROT_READ, MAP_SHARED, fd, 0));
    }
    close(fd);
  } else {
    XEV_ERROR("[ArxivMount] Can't open arxPath for mmap: {}", arxPath.string());
  }
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
  std::vector<std::string> paths;
  std::vector<char> buffer(64 * 1024);  // 64kB

  // write payloads
  for (const auto& dirEntry :
       std::filesystem::recursive_directory_iterator(targetDir)) {
    if (!dirEntry.is_regular_file())
      continue;

    // avoid packing itself
    if (std::filesystem::equivalent(dirEntry.path(), writeFile)) {
      continue;
    }

    // avoid dotfiles
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

    // write file content
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
    paths.emplace_back(relPathStr);
  }

  // write entries
  uint64_t entryOffset = static_cast<uint64_t>(out.tellp());
  out.write(reinterpret_cast<const char*>(entries.data()),
            entries.size() * sizeof(ARXEntry));

  // write paths
  uint64_t offsetTranslation = static_cast<uint64_t>(out.tellp());
  for (const auto& p : paths) {
    out.write(p.c_str(), p.size() + 1);
  }
  uint64_t sizeTranslation =
      static_cast<uint64_t>(out.tellp()) - offsetTranslation;

  // write header
  header.numEntry = static_cast<uint32_t>(entries.size());
  header.offsetEntry = entryOffset;
  header.sizeTranslation = sizeTranslation;
  header.offsetTranslation = offsetTranslation;
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
  if (!m_mmap)
    return {};

  uint64_t hash = StringHash{}(path);
  auto it = m_entries.find(hash);
  if (it == m_entries.end()) {
    return {};
  }

  const ARXEntry& entry = it->second;
  if (offset >= entry.sizeOriginal) {
    return {};
  }

  uint64_t toRead =
      (count == fs::EOF_COUNT) ? (entry.sizeOriginal - offset) : count;
  toRead = std::min(toRead, entry.sizeOriginal - offset);

  return {m_mmap + entry.offset + offset, toRead};
}

}  // namespace xev
