#include <tiny_gltf.h>
#include <xev/filesystem/fs.h>
#include <xev/filesystem/mount.h>
#include <xev/thread.h>
#include <xev/logger.h>

namespace xev {

FileSystem::FileSystem() {}

FileSystem::~FileSystem() {}

void FileSystem::init_thread_pool(uint32_t numThreads) const {
  m_threadPool = std::make_unique<ThreadPool>(numThreads);
}

void FileSystem::destroy_thread_pool() {
  m_threadPool.reset();
}

// sorry for the ugly template code...
template <std::derived_from<Mount> T>
void FileSystem::mount(T&& mnt) {
  uint32_t mnt_idx = static_cast<uint32_t>(mnts.size());
  for (const auto& [path, _] : mnt.directive) {
    directive[path] = mnt_idx;
  }
  mnts.push_back(std::make_unique<std::remove_cvref_t<T>>(std::forward<T>(mnt)));
}

uint32_t FileSystem::find_mnt(std::string_view filepath) const {
  // search for indexed sources
  auto it = directive.find(std::string(filepath));
  if (it != directive.end())
    return it->second.advance;

  return 0;
}

std::vector<uint8_t> FileSystem::read(std::string_view filepath,
                                      uint64_t offset,
                                      uint64_t count) const {
  uint32_t idx = find_mnt(filepath);
  if (idx == 0)
    XEV_ERROR("Failed to find file");
  return mnts[idx - 1]->read(filepath, offset, count);
}

std::future<std::vector<uint8_t>> FileSystem::read_async(
    std::string_view filepath,
    uint64_t offset,
    uint64_t count) const {
  uint32_t idx = find_mnt(filepath);
  if (idx == 0)
    XEV_ERROR("Failed to find file");

  if (m_threadPool == nullptr)
    init_thread_pool();

  return m_threadPool->submit(
    [this, p = std::string(filepath), offset, count]() {
      return this->read(p, offset, count);
    }
  );
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
