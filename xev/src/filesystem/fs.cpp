#include <tiny_gltf.h>
#include <xev/filesystem/fs.h>
#include <xev/filesystem/mount.h>
#include <xev/logger.h>
#include <xev/thread.h>

namespace xev {

FileSystem::FileSystem() {}

FileSystem::~FileSystem() {}

void FileSystem::init_thread_pool(uint32_t numThreads) const {
  m_threadPool = std::make_unique<ThreadPool>(numThreads);
}

void FileSystem::destroy_thread_pool() {
  m_threadPool.reset();
}

// IMPORTANT: This always return idx + 1
// since
uint32_t FileSystem::find_mnt(std::string_view filepath) const {
  auto it = directive.find(filepath);
  if (it != directive.end()) {
    return it->second;
  }
  return fs::INVALID_MOUNT;
}

std::vector<uint8_t> FileSystem::read(std::string_view filepath,
                                      uint64_t offset,
                                      uint64_t count) const {
  uint32_t idx = find_mnt(filepath);
  if (idx == fs::INVALID_MOUNT || idx >= mnts.size()) {
    XEV_ERROR("Failed to find file {}", filepath);
    return {};
  }
  return mnts[idx]->read(filepath, offset, count);
}

std::future<std::vector<uint8_t>> FileSystem::read_async(
    std::string_view filepath,
    uint64_t offset,
    uint64_t count) const {
  if (m_threadPool == nullptr)
    init_thread_pool();

  return m_threadPool->submit(
      [this, p = std::string(filepath), offset, count]() {
        return this->read(p, offset, count);
      });
}

void FileSystem::attach(tinygltf::TinyGLTF& loader) const {
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
