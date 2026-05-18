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

  Buffer(VkBufferUsageFlags flags_)
      : size(0), flags(flags_), usage(VMA_MEMORY_USAGE_AUTO) {}
  Buffer(VkDeviceSize size_, VkBufferUsageFlags flags_, VmaMemoryUsage usage_)
      : size(size_), flags(flags_), usage(usage_) {}

  uint64_t size_device() const override { return size; }

  bool is_reserved() const override { return (buffer == VK_NULL_HANDLE); };
  void reserve(const Backend& backend) override;
  void reserve(VkDeviceSize size_, const Backend& backend);
  void release(const Backend& backend) override;
  void upload(const void* src,
              uint64_t offset,
              uint64_t size_,
              const Backend& backend);
};

}  // namespace xev
