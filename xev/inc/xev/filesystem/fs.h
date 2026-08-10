#include <atomic>
#include <future>
#include <queue>
#include <semaphore>
#include <span>
#include <string>
#include <string_view>
#include <unique_ptr>
#include <unordered_map>
#include <vector>

namespace xev {

struct FileSource;
struct ThreadPool;

enum SourcePriority : uint32_t {
  LoosePriority = 0,
  ArcivPriority = 1,
};

constexpr uint64_t eofCount = static_cast<uint64_t>(-1);

struct FileSystem {
  FileSystem();
  ~FileSystem();

  bool is_indexed{false};
  std::unordered_map<std::string, uint32_t> directive;
  std::vector<std::unique_ptr<FileSource>> srcs;

  void init_thread_pool(uint32_t numThreads = 4);
  void destroy_thread_pool();

  void mount(std::unique_ptr<FileSource> src);
  void index();
  void index(uint32_t srcIdx);
  void find_src(std::string_view filepath);

  bool exists(std::string_view path) const;
  std::vector<uint8_t> read(std::string_view path,
                            uint64_t offset = 0,
                            uint64_t count = eofCount) const;
  void read(std::string_view path,
            uint64_t offset = 0,
            std::vector<uint8_t>& tgt) const;
  std::future<std::vector<uint8_t>> read_async(std::string_view path,
                                               uint64_t offset = 0,
                                               uint64_t count = eofCount) const;

 private:
  mutable std::unique_ptr<ThreadPool> m_threadPool;
};

}  // namespace xev
