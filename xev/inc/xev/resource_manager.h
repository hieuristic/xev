#pragma once
#include <xev/vma.h>
#include <xev/volk.h>
#include <xev/resource/sampler.h>
#include <string>
#include <vector>

namespace xev {

struct Buffer;
struct Image;
struct HotExec;
struct FileSystem;

struct ResourceManager {
  ResourceManager(VkInstance instance,
                  VkPhysicalDevice physical_device,
                  VkDevice device,
                  const FileSystem& fileSys);
  ~ResourceManager();

  ResourceManager(const ResourceManager&) = delete;
  ResourceManager& operator=(const ResourceManager&) = delete;
  ResourceManager(ResourceManager&&) = default;
  ResourceManager& operator=(ResourceManager&&) = delete;

  void alloc(Image& img) const;
  void free(Image& img) const;
  void load(Image& img, std::string path) const;
  void upload(const HotExec& hotExec,
              const std::vector<Buffer>& dsts,
              const std::vector<void*>& srcs,
              const std::vector<uint64_t>& sizes) const;

  void alloc(Buffer& buf) const;
  void free(Buffer& buf) const;

  void alloc(Sampler& sampler) const;
  void free(Sampler& sampler) const;

 private:
  VkDevice m_device{VK_NULL_HANDLE};
  VmaAllocator m_allocator{nullptr};

  const FileSystem& m_fileSys;

  void init_allocator(VkInstance instance,
                      VkPhysicalDevice physical_device,
                      VkDevice device);
};

}  // namespace xev
