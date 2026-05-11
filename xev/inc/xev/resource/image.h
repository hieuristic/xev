#pragma once
#include <xev/resource/resource.h>
#include <xev/vma.h>
#include <xev/volk.h>
#include <limits>

namespace xev {

class Backend;

class Image : Resource {
 public:
  Image(VkFormat format_, VkImageUsageFlags flags_)
      : format(format_), flags(flags_) {}
  Image(uint32_t width_,
        uint32_t height_,
        VkFormat format_,
        VkImageUsageFlags flags_)
      : width(width_), height(height_), format(format_), flags(flags_) {}

  VkImage image = VK_NULL_HANDLE;
  VkImageView view = VK_NULL_HANDLE;
  VmaAllocation alloc;
  VmaAllocationInfo alloc_info;

  uint32_t width = 0;
  uint32_t height = 0;
  VkFormat format;
  VkImageUsageFlags flags;

  uint64_t size_device() const override { return alloc_info.size; }
  uint64_t size_host() const override { return 0; }

  bool is_loaded() const override { return image != VK_NULL_HANDLE; }

  void load(uint32_t width_, uint32_t height_, const Backend& backend);
  void load(const Backend& backend) override;
  void unload(const Backend& backend) override;
};

}  // namespace xev
