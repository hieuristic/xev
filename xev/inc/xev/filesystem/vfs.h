#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace xev {

struct FileSource;

struct VirtualFileSystem {
  VFS();
  ~VFS();

  bool is_indexed{false};
  std::unordered_map<std::string, uint32_t> directive;
  std::vector<FileSource> srcs;

  void init_thread_pool(uint32_t numThreads = 4);
  void destroy_thread_pool();

  void mount(FileSource src);
  void index();
  void index(uint32_t srcIdx);

  bool exists(std::string_view path) const;
  [[nodiscard]] std::vector<uint8_t> read(std::string_view path) const;
  [[nodiscard]] void read_async(std::string_view path, std::mutex ) const;
};

}  // namespace xev
