#pragma once
#include <xev/vma.h>
#include <xev/volk.h>

namespace xev {

class ResourceManager {
 public:
  ResourceManager(VkInstance instance,
                  VkPhysicalDevice physical_device,
                  VkDevice device);
  ~ResourceManager();

  ResourceManager(const ResourceManager&) = delete;
  ResourceManager& operator=(const ResourceManager&) = delete;
  ResourceManager(ResourceManager&&) = default;
  ResourceManager& operator=(ResourceManager&&) = default;

  void reserve_buffer(VkBuffer& buffer,
                      VmaAllocation& alloc,
                      const VmaAllocationInfo& alloc_info,
                      VkDeviceSize size,
                      const VkBufferUsageFlags& flags,
                      VmaMemoryUsage mem_usage) const;
  void release_buffer(VkBuffer buffer, VmaAllocation alloc) const;
  void upload_buffer(const void* src,
                     const VmaAllocation& dst_alloc,
                     uint64_t offset,
                     uint64_t size) const;

  void reserve_image(VkImage& image,
                     VkImageView& view,
                     VmaAllocation& alloc,
                     VmaAllocationInfo& alloc_info,
                     uint32_t width,
                     uint32_t height,
                     VkFormat format,
                     VkImageUsageFlags flags) const;
  void release_image(VkImage& image,
                     VmaAllocation& alloc,
                     VkImageView& view) const;

 private:
  VkDevice m_device{VK_NULL_HANDLE};
  VmaAllocator m_allocator{nullptr};

  void init_allocator();
};

}  // namespace xev
