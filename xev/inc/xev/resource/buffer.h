#pragma once
#include <SDL3/SDL.h>
#include <xev/resource/resource.h>
#include <xev/vma.h>

namespace xev {

class Backend;

class Buffer : Resource {
 public:
  Buffer(VkBufferUsageFlags flags_) : size(0), flags(flags_) {}
  Buffer(VkDeviceSize size_, VkBufferUsageFlags flags_, VmaMemoryUsage usage_)
      : size(size_), flags(flags_), usage(usage_) {}

  VkBuffer buffer{VK_NULL_HANDLE};
  VmaAllocation alloc{VK_NULL_HANDLE};
  VmaAllocationInfo alloc_info{};
  VkBufferUsageFlags flags{0};
  VmaMemoryUsage usage{VMA_MEMORY_USAGE_AUTO};
  VkDeviceSize size{0};

  uint64_t size_device() const override { return size; }

  bool on_device() const override { return buffer != VK_NULL_HANDLE; }
};

}  // namespace xev
