#pragma once
#include <SDL3/SDL.h>
#include <xev/resource/resource.h>
#include <xev/vma.h>

namespace xev {

class Backend;

class Buffer : Resource {
 public:
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceSize size;
  VkBufferUsageFlags flags;
  VmaMemoryUsage usage;
  VmaAllocation alloc;
  VmaAllocationInfo alloc_info;

  Buffer(VkDeviceSize size_, VkBufferUsageFlags flags_, VmaMemoryUsage usage_)
      : size(size_), flags(flags_), usage(usage_) {}

  uint64_t size_device() const override { return size; }
  uint64_t size_host() const override { return 0; }

  bool is_loaded() const override { return (buffer == VK_NULL_HANDLE); };
  void load(const Backend& backend) override;
  void load(VkDeviceSize size_, const Backend& backend);
  void unload(const Backend& backend) override;
  void copy(const void* src,
            uint64_t offset,
            uint64_t size_,
            const Backend& backend);
};

}  // namespace xev
