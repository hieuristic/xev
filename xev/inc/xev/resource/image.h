#pragma once
#include <xev/resource/resource.h>
#include <xev/vma.h>
#include <xev/volk.h>
#include <limits>

namespace xev {

class ResourceManager;

class Image : Resource {
 public:
  Image(VkFormat format_, VkImageUsageFlags flags_)
      : format(format_), flags(flags_) {}
  Image(uint32_t width_,
        uint32_t height_,
        VkFormat format_,
        VkImageUsageFlags flags_)
      : width(width_), height(height_), format(format_), flags(flags_) {}
  Image(VkImage image_,
        VkImageView view_,
        uint32_t width_,
        uint32_t height_,
        VkFormat format_,
        VkImageUsageFlags flags_)
      : image(image_),
        view(view_),
        width(width_),
        height(height_),
        format(format_),
        flags(flags_) {}

  VkImage image{VK_NULL_HANDLE};
  VkImageView view{VK_NULL_HANDLE};
  VmaAllocation alloc{VK_NULL_HANDLE};
  VmaAllocationInfo alloc_info{};

  uint32_t width{0};
  uint32_t height{0};
  VkFormat format{VK_FORMAT_UNDEFINED};
  VkImageUsageFlags flags{0};

  uint64_t size_device() const override { return alloc_info.size; }
  bool is_reserved() const override { return image != VK_NULL_HANDLE; }
  void reserve(uint32_t width_,
               uint32_t height_,
               const ResourceManager& manager);
  void reserve(const ResourceManager& manager) override;
  void release(const ResourceManager& manager) override;
  void upload(const ResourceManager& manager);

  VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
  void set_layout(const VkImageLayout& layout_);
  void update_layout(const VkCommandBuffer& cmd,
                     const VkImageLayout& new_layout_);

  void copy(const VkCommandBuffer& cmd,
            const Image& src,
            const VkFilter& filter);
};

}  // namespace xev
