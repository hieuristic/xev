#pragma once
#include <SDL3/SDL.h>
#include <xev/vma.h>

namespace xev {

struct Buffer {
  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation alloc;
  VmaAllocationInfo alloc_info;
};

} // namespace xev
