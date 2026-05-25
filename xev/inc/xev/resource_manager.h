#pragma once
#include <xev/vma.h>
#include <xev/volk.h>
#include <xev/resource/sampler.h>

namespace xev {

class Buffer;
class Image;

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

  void alloc(Buffer& buf) const;
  void alloc(Image& img) const;
  void free(Buffer& buf) const;
  void free(Image& img) const;
  void upload(Buffer& buf, const void* src, uint64_t offset, uint64_t size) const;

  void alloc(VkSampler& sampler, SamplerType type) const;
  void free(VkSampler& sampler) const;

 private:
  VkDevice m_device{VK_NULL_HANDLE};
  VmaAllocator m_allocator{nullptr};

  void init_allocator(VkInstance instance,
                      VkPhysicalDevice physical_device,
                      VkDevice device);
};

}  // namespace xev
