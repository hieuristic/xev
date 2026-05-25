#include <xev/swapchain.h>
#include <xev/logger.h>

namespace xev {

Swapchain::Swapchain(VkPhysicalDevice physical_device,
                     VkSurfaceKHR surface,
                     VkDevice device,
                     uint32_t graphics_family_idx,
                     uint32_t present_family_idx)
    : m_device(device),
      m_physical_device(physical_device),
      m_surface(surface),
      m_graphics_family_idx(graphics_family_idx),
      m_present_family_idx(present_family_idx) {
  XEV_ASSERT(surface != VK_NULL_HANDLE);
  init_swapchain();
  create_images();
}

Swapchain::~Swapchain() {
  destroy_images();

  if (m_swapchain != VK_NULL_HANDLE)
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}

void Swapchain::init_swapchain() {
  VkResult res_;

  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_surface,
                                            &capabilities);

  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface,
                                       &format_count, nullptr);
  if (format_count == 0) {
    XEV_ERROR("No surface formats supported");
  }
  std::vector<VkSurfaceFormatKHR> formats(format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface,
                                       &format_count, formats.data());

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface,
                                            &present_mode_count, nullptr);
  if (present_mode_count == 0) {
    XEV_ERROR("No surface present modes supported");
  }
  std::vector<VkPresentModeKHR> present_modes(present_mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      m_physical_device, m_surface, &present_mode_count, present_modes.data());

  m_surface_format = formats[0];
  for (const auto& format : formats) {
    if (format.format == m_ideal_format.format &&
        format.colorSpace == m_ideal_format.colorSpace) {
      m_surface_format = format;
      break;
    }
  }

  if (m_surface_format.format != m_ideal_format.format ||
      m_surface_format.colorSpace != m_ideal_format.colorSpace) {
    XEV_ERROR("Ideal format not supported by swapchain");
  }

  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
  for (const auto& mode : present_modes) {
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      present_mode = mode;
      break;
    }
  }

  height = capabilities.currentExtent.height;
  width = capabilities.currentExtent.width;

  uint32_t image_count = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 &&
      image_count > capabilities.maxImageCount) {
    image_count = capabilities.maxImageCount;
  }

  XEV_INFO("Image count supported for swapchain: {}",
           capabilities.minImageCount);

  VkSwapchainCreateInfoKHR swapchain_info{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = m_surface,
      .minImageCount = image_count,
      .imageFormat = m_surface_format.format,
      .imageColorSpace = m_surface_format.colorSpace,
      .imageExtent = {width, height},
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .preTransform = capabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = present_mode,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE,
  };

  uint32_t queueFamilyIndices[] = {m_graphics_family_idx, m_present_family_idx};
  if (m_graphics_family_idx != m_present_family_idx) {
    swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_info.queueFamilyIndexCount = 2;
    swapchain_info.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  res_ = vkCreateSwapchainKHR(m_device, &swapchain_info, nullptr, &m_swapchain);
  XEV_ASSERT_VK(res_, "Swapchain creation failed: {}", (int)res_);
}

void Swapchain::create_images() {
  VkResult res_;
  uint32_t cnt;
  vkGetSwapchainImagesKHR(m_device, m_swapchain, &cnt, nullptr);
  std::vector<VkImage> images(cnt);
  vkGetSwapchainImagesKHR(m_device, m_swapchain, &cnt, images.data());

  m_images.reserve(cnt);
  for (uint32_t i = 0; i < cnt; i++) {
    VkImageViewCreateInfo view_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = images[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = m_surface_format.format,
        .components = {VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    VkImageView view;
    res_ = vkCreateImageView(m_device, &view_info, nullptr, &view);
    XEV_ASSERT_VK(res_, "Failed to create swapchain image view!");

    m_images.emplace_back(Image(images[i], view, width, height,
                                m_surface_format.format,
                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
  }
}

void Swapchain::destroy_images() {
  for (auto& image : m_images)
    vkDestroyImageView(m_device, image.view, nullptr);
  m_images.clear();
}

void Swapchain::reinit_swapchain() {
  vkDeviceWaitIdle(m_device);

  destroy_images();
  if (m_swapchain != VK_NULL_HANDLE)
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

  init_swapchain();
  create_images();
  m_is_swapchain_dirty = false;
}

const Image& Swapchain::acquire_image(VkSemaphore swapchain_sem) {
  VkResult res_;
  res_ = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, swapchain_sem,
                               VK_NULL_HANDLE, &m_acquired_idx);
  if (res_ == VK_ERROR_OUT_OF_DATE_KHR) {
    m_is_swapchain_dirty = true;
  } else if (res_ != VK_SUCCESS && res_ != VK_SUBOPTIMAL_KHR) {
    XEV_ASSERT_VK(res_, "Failed to acquire swapchain image");
  }
  return m_images[m_acquired_idx];
}

void Swapchain::present(VkQueue queue, VkSemaphore wait_sem) {
  VkPresentInfoKHR info = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &wait_sem,
      .swapchainCount = 1,
      .pSwapchains = &m_swapchain,
      .pImageIndices = &m_acquired_idx,
  };

  VkResult res_ = vkQueuePresentKHR(queue, &info);
  if (res_ == VK_ERROR_OUT_OF_DATE_KHR || res_ == VK_SUBOPTIMAL_KHR) {
    m_is_swapchain_dirty = true;
  } else if (res_ != VK_SUCCESS) {
    XEV_ERROR("Failed to present: {}", (int)res_);
  }
}

}  // namespace xev
