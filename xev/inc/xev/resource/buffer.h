#pragma once
#include <SDL3/SDL.h>
#include <xev/resource/resource.h>
#include <xev/vma.h>

namespace xev {

class Backend;

class Buffer : Resource {
 public:
  Buffer(VkBufferUsageFlags flags_) : flags(flags_) {}
  Buffer(VkBufferUsageFlags flags_, VmaMemoryUsage usage_)
      : flags(flags_), usage(usage_) {}
  Buffer(uint64_t size_, VkBufferUsageFlags flags_, VmaMemoryUsage usage_)
      : size(size_), flags(flags_), usage(usage_) {}

  VkDeviceAddress addr{0};
  VkBuffer buffer{VK_NULL_HANDLE};
  VmaAllocation alloc{VK_NULL_HANDLE};
  VmaAllocationInfo alloc_info{};
  VkBufferUsageFlags flags{0};
  VmaMemoryUsage usage{VMA_MEMORY_USAGE_AUTO};
  uint64_t size{0};

  uint64_t size_device() const override { return size; }
  bool on_device() const override { return buffer != VK_NULL_HANDLE; }

  void upload(VkCommandBuffer cmdbuf,
              const Buffer& staging_buffer,
              void* src,
              uint64_t size,
              uint64_t offset);
};

}  // namespace xev
