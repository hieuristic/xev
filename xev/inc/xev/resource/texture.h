#pragma once
#include <xev/resource/image.h>
#include <xev/resource/resource.h>
#include <xev/resource/sampler.h>

namespace xev {

class Texture : Resource {
 public:
  Texture(std::string name_, VkFormat format_, VkImageUsageFlags flags_)
      : name(name_), image(format_, flags_) {}
  Texture(std::string name_,
          uint32_t width_,
          uint32_t height_,
          VkFormat format_,
          VkImageUsageFlags flags_)
      : name(name_), image(width_, height_, format_, flags_) {}

  Image image;
  Sampler sampler;
  std::vector<uint8_t> raw_data;
  std::string name;

  uint64_t size_device() const override;
  bool is_reserved() const override;
  void reserve(uint32_t width_, uint32_t height_, const Backend& backend);
  void reserve(const Backend& backend) override;
  void release(const Backend& backend) override;
  void upload(const Backend& backend);

  VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
  void set_layout(const VkImageLayout& layout_);
  void update_layout(const VkCommandBuffer& cmd,
                     const VkImageLayout& new_layout_);

  void copy(const VkCommandBuffer& cmd,
            const Image& src,
            const VkFilter& filter);
};

}  // namespace xev
