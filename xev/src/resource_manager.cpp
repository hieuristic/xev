#include <xev/resource_manager.h>

namespace xev {

ResourceManager::ResourceManager(VkInstance instance,
                                 VkPhysicalDevice physical_device,
                                 VkDevice device)
    : m_device(device) {
  init_allocator(instance, physical_device, device);
}

ResourceManager::~ResourceManager() {
  if (m_allocator != nullptr)
    vmaDestroyAllocator(m_allocator);
}

void ResourceManager::init_allocator(VkInstance instance,
                                     VkPhysicalDevice physical_device,
                                     VkDevice device) {
  VmaAllocatorCreateInfo info = {
      .flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT |
               VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT |
               VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT,
      .physicalDevice = physical_device,
      .device = device,
      .instance = instance,
      .vulkanApiVersion = VK_API_VERSION_1_3,
  };

  VmaVulkanFunctions vma_func{};
  vma_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vma_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
  info.pVulkanFunctions = &vma_func;

  VkResult res_ = vmaCreateAllocator(&info, &m_allocator);
  XEV_ASSERT_VK(res_, "Failed to create VMA");
}

void ResourceManager::reserve_buffer(VkBuffer& buffer,
                                     VmaAllocation& alloc,
                                     VmaAllocationInfo& alloc_info,
                                     VkDeviceSize size,
                                     VkBufferUsageFlags flags,
                                     VmaMemoryUsage mem_usage) const {
  VkBufferCreateInfo buffer_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = flags,
  };

  VmaAllocationCreateInfo create_info = {
      .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
               VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
      .usage = mem_usage,
  };

  VkResult res_ = vmaCreateBuffer(m_allocator, &buffer_info, &create_info,
                                  &buffer, &alloc, &alloc_info);

  XEV_ASSERT_VK(res_, "Failed to create buffer");
}

void ResourceManager::release_buffer(VkBuffer buffer,
                                     VmaAllocation alloc) const {
  vmaDestroyBuffer(m_allocator, buffer, alloc);
}

void ResourceManager::upload_buffer(const void* src,
                                    const VmaAllocation& dst_alloc,
                                    uint64_t offset,
                                    uint64_t size) const {
  vmaCopyMemoryToAllocation(m_allocator, src, dst, offset, size);
}

void ResourceManager::reserve_image(VkImage& image,
                                    VmaAllocation& alloc,
                                    VmaAllocationInfo& alloc_info,
                                    uint32_t width,
                                    uint32_t height,
                                    VkFormat format,
                                    VkImageUsageFlags flags) const {
  VkImageCreateInfo image_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = format,
      .extent = {width, height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = flags,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VmaAllocationCreateInfo create_info = {
      .usage = VMA_MEMORY_USAGE_GPU_ONLY,
  };

  VkResult res_ = vmaCreateImage(m_allocator, &image_info, &create_info, &image,
                                 &alloc, &alloc_info);
  XEV_ASSERT_VK(res_, "Failed to allocate image");

  VkImageAspectFlags aspect_mask = 0;
  if (flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
  if (flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
  if (flags & VK_IMAGE_USAGE_SAMPLED_BIT && aspect_mask == 0)
    aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;

  VkImageViewCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = format,
      .subresourceRange =
          {
              .aspectMask = aspect_mask,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };

  VkResult res_ = vkCreateImageView(m_device, &info, nullptr, &view);
  XEV_ASSERT_VK(res_, "Failed to create image view");
}

void ResourceManager::release_image(VkImage& image,
                                    VmaAllocation& alloc) const {
  if (view != VK_NULL_HANDLE) {
    vkDestroyImageView(m_device, view, nullptr);
    view = VK_NULL_HANDLE;
  }
  if (image != VK_NULL_HANDLE) {
    vmaDestroyImage(m_allocator, image, alloc);
    image = VK_NULL_HANDLE;
  }
}

}  // namespace xev
