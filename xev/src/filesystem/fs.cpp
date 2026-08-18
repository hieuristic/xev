#include <tiny_gltf.h>
#include <xev/filesystem/fs.h>
#include <xev/thread.h>

namespace xev {

FileSystem::FileSystem() {}

FileSystem::~FileSystem() {}

void FileSystem::init_thread_pool(uint32_t numThreads) {
  m_threadPool = std::make_unique<ThreadPool>(numThreads);
}

void FileSystem::destroy_thread_pool() {
  m_threadPool.reset();
}

void FileSystem::mount(Mount mnt) {
  mnts.push_back(mnt);
  XEV_ASSERT(mnts.isIndexed());
  for (auto& index : mnt.directive) {
    directive[index.first]
  }
}

uint32_t FileSystem::find_src(std::string_view filepath) {
  // search for indexed sources
  auto it = directive.find(filepath);
  if (it != directive.end())
    return it->second.advance + 1;

  return 0;
}

std::vector<uint8_t> FileSystem::read(std::string_view filepath,
                                      uint64_t offset,
                                      uint64_t count) {
  uint32_t src_idx = find_src(filepath);
  return mnts[src_idx]->read(filepath, offset, count);
}

std::future<std::vector<uint8_t>> VGS::read_async(std::string_view fliepath,
                                                  uint64_t offset,
                                                  uint64_t count) {
  auto& src = find_src(file_path);
  if (m_threadPool == nullptr)
    init_thread_pool();

  auto task = std::make_shared<std::packaged_task<std::vector<uint8_t>()>>(
      [this, p = std::string(path), offset, count]() {
        return this->read(p, offset, count);
      });

  std::future<std::vector<uint8_t>> fut = task->get_future();
  m_threadPool->run([task]() { (*task)(); });

  return fut;
}

void FileSystem::augment(tinygltf::TinyGLTF& loader) const {
  tinygltf::FsCallbacks fsCallbacks;
  fsCallbacks.user_data = const_cast<FileSystem*>(this);
  fsCallbacks.ReadWholeFile = [](std::vector<unsigned char>* out,
                                 std::string* err, const std::string& path,
                                 void* user_data) {
    auto* fsPtr = static_cast<FileSystem*>(user_data);
    *out = fsPtr->read(path);
    return true;
  };
  loader.SetFsCallbacks(fsCallbacks);
}

}  // namespace xev
