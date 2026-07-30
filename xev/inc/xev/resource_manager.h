#pragma once
#include <xev/vma.h>
#include <xev/volk.h>
#include <xev/resource/sampler.h>
#include <string>

namespace xev {

class Buffer;
class Image;
class HotExec;

class ResourceManager {
 public:
  ResourceManager(VkInstance instance,
                  VkPhysicalDevice physical_device,
                  VkDevice device);
  ~ResourceManager();

  ResourceManager(const ResourceManager&) = delete;
  ResourceManager& operator=(const ResourceManager&) = delete;
  ResourceManager(ResourceManager&&) = default;
  ResourceManager& operator=(ResourceManager&&) = default;

  void alloc(Image& img) const;
  void free(Image& img) const;
  void load(Image& img, std::string path);
  void upload(const HotExec& hot_exec,
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

  void init_allocator(VkInstance instance,
                      VkPhysicalDevice physical_device,
                      VkDevice device);
};

}  // namespace xev
