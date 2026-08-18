#include <xev/hot_exec.h>
#include <xev/logger.h>
#include <xev/resource/buffer.h>
#include <xev/resource/image.h>
#include <xev/resource/sampler.h>
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
               VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT |
               VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
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

void ResourceManager::alloc(Buffer& buf) const {
  XEV_ASSERT(buf.size != 0, "Trying to allocate an empty buffer.");

  VkBufferCreateInfo buffer_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = buf.size,
      .usage = buf.flags,
  };

  VmaAllocationCreateInfo create_info = {
      .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
               VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
      .usage = buf.usage,
  };

  VkResult res_ = vmaCreateBuffer(m_allocator, &buffer_info, &create_info,
                                  &buf.buffer, &buf.alloc, &buf.alloc_info);

  XEV_ASSERT_VK(res_, "Failed to create buffer");

  if (buf.flags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
    VkBufferDeviceAddressInfo info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buf.buffer};
    buf.addr = vkGetBufferDeviceAddress(m_device, &info);
  }
}

void ResourceManager::free(Buffer& buf) const {
  vmaDestroyBuffer(m_allocator, buf.buffer, buf.alloc);
}

void ResourceManager::upload(const HotExec& hotExec,
                             const std::vector<Buffer>& dsts,
                             const std::vector<void*>& srcs,
                             const std::vector<uint64_t>& sizes) const {
  uint64_t sum_size = 0;
  for (uint32_t i = 0; i < sizes.size(); i++)
    sum_size += sizes[i];
  Buffer staging{sum_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VMA_MEMORY_USAGE_AUTO};
  alloc(staging);

  {
    uint64_t offset_ = 0;
    for (uint32_t i = 0; i < dsts.size(); i++) {
      memcpy((char*)staging.alloc_info.pMappedData + offset_, srcs[i],
             sizes[i]);
      offset_ += sizes[i];
    }
  }

  hotExec.run([&](const VkCommandBuffer cmdbuf) {
    uint64_t offset_ = 0;
    for (uint32_t i = 0; i < dsts.size(); i++) {
      const VkBufferCopy reg = {
          .srcOffset = offset_,
          .dstOffset = 0,
          .size = sizes[i],
      };
      vkCmdCopyBuffer(cmdbuf, staging.buffer, dsts[i].buffer, 1, &reg);
      offset_ += sizes[i];
    }
  });

  free(staging);
}

void ResourceManager::alloc(Image& img) const {
  XEV_ASSERT(img.width != 0 && img.height != 0);

  VkImageCreateInfo image_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = img.format,
      .extent = {img.width, img.height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = img.flags,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VmaAllocationCreateInfo create_info = {
      .usage = VMA_MEMORY_USAGE_GPU_ONLY,
  };

  VkResult res_ = vmaCreateImage(m_allocator, &image_info, &create_info,
                                 &img.image, &img.alloc, &img.alloc_info);
  XEV_ASSERT_VK(res_, "Failed to allocate image");

  VkImageAspectFlags aspect_mask = 0;
  if (img.flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
  if (img.flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
  if (img.flags & VK_IMAGE_USAGE_SAMPLED_BIT && aspect_mask == 0)
    aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;

  VkImageViewCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = img.image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = img.format,
      .subresourceRange =
          {
              .aspectMask = aspect_mask,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };

  res_ = vkCreateImageView(m_device, &info, nullptr, &img.view);
  XEV_ASSERT_VK(res_, "Failed to create image view");
}

void ResourceManager::free(Image& img) const {
  if (img.view != VK_NULL_HANDLE) {
    vkDestroyImageView(m_device, img.view, nullptr);
    img.view = VK_NULL_HANDLE;
  }
  if (img.image != VK_NULL_HANDLE) {
    vmaDestroyImage(m_allocator, img.image, img.alloc);
    img.image = VK_NULL_HANDLE;
  }
}

void ResourceManager::load(Image& img, std::string path) const {
  alloc(img);

  std::vector<uint8_t> memory = m_fs.read(path);
  int w, h, c;
  stbi_uc* data = stbi_load_from_memory(
      memory.data(), static_cast<int>(memory.size()), &w, &h, &c, 4);

  if (data) {
    img.width = w;
    img.height = h;
    img.host_data.assign(data, data + (w * h * 4));
    stbi_image_free(data);
  };
}

}  // namespace xev
