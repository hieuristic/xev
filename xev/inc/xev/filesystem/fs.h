// This is the file system interface that abstract away differences sources
// such as memory mapped assets, loose files, archived files, etc.
// Since this struct manage its own thread for concurency,
// it is NOT thread-safe. TODO: make it thread safe
#include <xev/filesystem/const.h>
#include <xev/util/string_hash.h>
#include <atomic>
#include <concepts>
#include <future>
#include <memory>
#include <queue>
#include <semaphore>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tinygltf {
struct TinyGLTF;
}

namespace xev {

struct Mount;
struct ThreadPool;

enum SourcePriority : uint32_t {
  LoosePriority = 0,
  ArcivPriority = 1,
};

struct FileSystem {
  FileSystem();
  ~FileSystem();

  // NO COPY!
  FileSystem(const FileSystem&) = delete;
  FileSystem& operator=(const FileSystem&) = delete;

  // Moving allowed
  FileSystem(FileSystem&&) = default;
  FileSystem& operator=(FileSystem&&) = default;

  bool isIndexed{false};
  std::unordered_map<std::string, uint32_t, StringHash, std::equal_to<>>
      directive;

  std::vector<std::unique_ptr<Mount>> mnts;

  void init_thread_pool(uint32_t numThreads = 4) const;
  void destroy_thread_pool();

  template <std::derived_from<Mount> T>
  void mount(T&& mnt) {
    uint32_t mnt_idx = static_cast<uint32_t>(mnts.size());
    for (const auto& [path, _] : mnt.directive) {
      directive[path] = mnt_idx;
    }
    mnts.push_back(
        std::make_unique<std::remove_cvref_t<T>>(std::forward<T>(mnt)));
  }

  void index();
  void index(uint32_t mntIdx);
  uint32_t find_mnt(std::string_view filepath) const;

  bool exists(std::string_view path) const;
  std::vector<uint8_t> read(std::string_view path,
                            uint64_t offset = 0,
                            uint64_t count = fs::EOF_COUNT) const;
  void read(std::string_view path,
            std::vector<uint8_t>& tgt,
            uint64_t offset = 0) const;
  std::future<std::vector<uint8_t>> read_async(
      std::string_view path,
      uint64_t offset = 0,
      uint64_t count = fs::EOF_COUNT) const;

  // loaders: Some loader (eg. tinygltf) requires additional modification
  // from the filesystem in order to read directly from the filesystem's
  // memory. These functions attach the callback from the filesystem to
  // these loaders.
  void attach(tinygltf::TinyGLTF& loader) const;

 private:
  mutable std::unique_ptr<ThreadPool> m_threadPool;
};

}  // namespace xev
