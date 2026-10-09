#pragma once
#include <xev/vma.h>
#include <array>
#include <cstdint>

namespace xev {

struct Backend;

struct BufferArray : Resource {
  BufferArray(uint32_t numBuffer_, VkBufferUsageFlags flags_)
      : numBuffer(numBuffer_), flags(flags_) {}
  BufferArray(uint32_t numBuffer_,
              VkBufferUsageFlags flags_,
              VmaMemoryUsage usage_)
      : numBuffer(numBuffer_), flags(flags_), usage(usage_) {}
  BufferArray(uint32_t numBuffer_,
              uint64_t slotSize_,
              VkBufferUsageFlags flags_,
              VmaMemoryUsage usage_)
      : numBuffer(numBuffer_),
        slotSize(slotSize_),
        flags(flags_),
        usage(usage_) {}

  VkDeviceAddress addr{0};
  VkBuffer buffer{VK_NULL_HANDLE};
  VmaAllocation alloc{VK_NULL_HANDLE};
  VmaAllocationInfo alloc_info{};
  VkBufferUsageFlags flags{0};
  VmaMemoryUsage usage{VMA_MEMORY_USAGE_AUTO};
  uint64_t stride{0};
  uint64_t slotSize{0};
  uint32_t numBuffer = NBUF;

  // minimal size for stride
  static constexpr uint64_t MIN_ALIGNMENT = 64;

  void set_size(uint64_t slotSize_) { ; }

  VkDeviceAddress get_addr(uint32_t idx) const {
    return addr + static_cast<VkDeviceAddress>(idx * stride);
  }

  uint64_t size_device() const override { return size; }
  bool on_device() const override { return buffer != VK_NULL_HANDLE; }

  void write(const uint32_t idx,
             const void* data,
             const uint64_t size,
             const uint64_t offset);
};

}  // namespace xev
