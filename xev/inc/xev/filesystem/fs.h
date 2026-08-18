// This is the file system interface that abstract away differences sources
// such as memory mapped assets, loose files, archived files, etc.
// Since this struct manage its own thread for concurency,
// it is NOT thread-safe. TODO: make it thread safe
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

constexpr uint64_t eofCount = static_cast<uint64_t>(-1);

struct FileSystem {
  FileSystem();
  ~FileSystem();

  // NO COPY!
  FileSystem(const FileSystem&) = delete;
  FileSystem& operator=(const FileSystem&) = delete;

  // Moving allowed
  FileSystem(FileSystem&&) = default;
  FileSystem& operator=(FileSystem&&) = default;

  bool is_indexed{false};
  std::unordered_map<std::string, uint32_t> directive;
  std::vector<std::unique_ptr<Mount>> mnts;

  void init_thread_pool(uint32_t numThreads = 4) const;
  void destroy_thread_pool();

  template <std::derived_from<Mount> T>
  void mount(T&& mnt);

  void index();
  void index(uint32_t mntIdx);
  uint32_t find_mnt(std::string_view filepath) const;

  bool exists(std::string_view path) const;
  std::vector<uint8_t> read(std::string_view path,
                            uint64_t offset = 0,
                            uint64_t count = eofCount) const;
  void read(std::string_view path,
            std::vector<uint8_t>& tgt,
            uint64_t offset = 0) const;
  std::future<std::vector<uint8_t>> read_async(std::string_view path,
                                               uint64_t offset = 0,
                                               uint64_t count = eofCount) const;

  // loaders: Some loader (eg. tinygltf) requires additional modification
  // from the filesystem in order to read directly from the filesystem's
  // memory. These functions augments loaders.
  void augment(tinygltf::TinyGLTF& loader) const;

 private:
  mutable std::unique_ptr<ThreadPool> m_threadPool;
};

}  // namespace xev
