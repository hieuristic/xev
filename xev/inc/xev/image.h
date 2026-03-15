#pragma once

namespace xev {

class Image {
 public:
  VkImage image;
  VkImageView view;
  VmaAllocation alloc;
  VkFormat format;
  VkImageUsageFlags usage;

  bool is_valid() { return m_valid; }

 private:
  static const uint32_t INVALID_ID = std::numeric_limits<uint32_t>::max();
  uint32_t m_valid{INVALID_ID};
};

}  // namespace xev
