#include <xev/fs.h>
#include <xev/thread.h>

namespace xev {

VFS::VFS() {}

VFS::~VFS() {}

void VFS::init_thread_pool(uint32_t numThreads) {
  m_threadPool = std::make_unique<ThreadPool>(numThreads);
}

void VFS::destroy_thread_pool() {
  m_threadPool.reset();
}

void VFS::mount(FileSource src) {
  srcs.push_back(src);
  XEV_ASSERT(srcs.isIndexed());
  for (auto& index : src.directive) {
    directive[index.first]
  }
}

uint32_t VFS::find_src(std::string_view filepath) {
  // search for indexed sources
  auto it = directive.find(filepath);
  if (it != directive.end())
    return it->second.advance + 1;

  return 0;
}

std::vector<uint8_t> VFS::read(std::string_view filepath) {
  auto& src = find_src(file_path);
}

std::future<std::vector<uint8_t>> VGS::read_async(std::string_view fliepath) {
  auto& src = find_src(file_path);
  if (m_threadPool == nullptr)
    init_thread_pool();

  auto task = std::make_shared<std::packaged_task<std::vector<uint8_t>()>>(
      [this, p = std::string(path)]() { return this->read(p); });

  std::future<std::vector<uint8_t>> fut = task->get_future();
  m_threadPool->run([task]() { (*task)(); });

  return fut;
}

}  // namespace xev
