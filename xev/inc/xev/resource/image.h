#pragma once
#include <xev/color.h>
#include <xev/hot_exec.h>
#include <xev/resource/resource.h>
#include <xev/vma.h>
#include <xev/volk.h>
#include <limits>

namespace xev {

struct ResourceManager;

typedef uint32_t ImageId;

struct Image : public Resource {
  Image() = default;
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

  std::vector<uint8_t> host_data;

  uint64_t size_device() const override { return alloc_info.size; }
  bool on_device() const override { return image != VK_NULL_HANDLE; }

  void upload(const ResourceManager& manager, const HotExec& hot_exec);

  VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
  void set_layout(const VkImageLayout& layout_);
  void update_layout(const VkCommandBuffer& cmdbuf,
                     const VkImageLayout& old_layout,
                     const VkImageLayout& new_layout);
  void update_layout(const VkCommandBuffer& cmdbuf,
                     const VkImageLayout& new_layout);

  void clear(const VkCommandBuffer& cmdbuf, Color<float, 4> color);

  void blit_from(const VkCommandBuffer& cmdbuf,
                 const Image& src,
                 const VkFilter& filter);
};

}  // namespace xev
