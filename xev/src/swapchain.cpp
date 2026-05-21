#include <xev/m_device.h>
#include <xev/swapchain.h>

namespace xev {
Swapchain::Swapchain(VkPhysicalDevice physical_device,
                     VkSurfaceKHR surface,
                     VkDevice device,
                     QueueFamily queue_family)
    : m_physical_device(physical_device),
      m_device(device),
      m_surface(surface) m_queue_family(queue_family),
{
  XEV_ASSERT(surface != VK_NULL_HANDLE);
  init_swapchain();
  create_images();
  for (auto& frame : m_frames)
    create_frame(frame);
}

Swapchain::~Swapchain() {
  for (auto& frame : m_frames)
    Swapchain::destroy_frame(frame);

  destroy_images();

  if (m_swapchain != VK_NULL_HANDLE)
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}

void Swapchain::init_swapchain() {
  VkResult res_;
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;

  if (!m_queue_family.isComplete()) {
    XEV_ERROR("Queue families incomplete for swapchain");
    return;
  }

  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_surface,
                                            &capabilities);

  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface,
                                       &format_count, nullptr);
  if (format_count == 0) {
    XEV_ERROR("No m_surface formats supported");
  }
  std::vector<VkSurfaceFormatKHR> formats(format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface,
                                       &format_count, formats.data());

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface,
                                            &present_mode_count, nullptr);
  if (present_mode_count == 0) {
    XEV_ERROR("No m_surface present modes supported");
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

  if (surface_format.format != m_ideal_format.format ||
      surface_format.colorSpace != m_ideal_format.colorSpace) {
    XEV_ERROR("Ideal format not supported by swapchain");
  }

  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
  for (const auto& mode : present_modes) {
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      present_mode = mode;
      break;
    }
  }

  m_extent = capabilities.currentExtent;

  uint32_t image_count = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 &&
      image_count > capabilities.maxImageCount) {
    image_count = capabilities.maxImageCount;
  }

  XEV_INFO("image count supported for swapchain {}",
           capabilities.minImageCount);

  VkSwapchainCreateInfoKHR swapchain_info{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .m_surface = m_surface,
      .minImageCount = image_count,
      .imageFormat = surface_format.format,
      .imageColorSpace = surface_format.colorSpace,
      .imageExtent = m_extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .preTransform = capabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = present_mode,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE,
  };

  uint32_t queue_family_indices[] = {m_queue_family.graphics.value().idx,
                                     m_queue_family.pre.value().idx};
  if (m_queue_family.graphics.value().idx != m_queue_family.pre.value().idx) {
    swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_info.queueFamilyIndexCount = 2;
    swapchain_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  res_ = vkCreateSwapchainKHR(m_device, &swapchain_info, nullptr, &m_swapchain);
  XEV_ASSERT_VK(res_, "Swapchain creation failed: {}", (int)res_);
}

void Swapchain::create_images() {
  uint32_t cnt;
  vkGetSwapchainImagesKHR(m_device, m_swapchain, &cnt, nullptr);
  std::vector<VkImage> images(cnt);
  vkGetSwapchainImagesKHR(m_device, m_swapchain, &cnt, images.data());

  m_images.reserve(cnt);
  for (uint32_t i = 0; i < cnt; i++) {
    VkImageViewCreateInfo view_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_images[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = surface_format.format,
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

    m_images.emplace_back(Image(images[i], view, m_extent.width,
                                m_extent.height, VK_FORMAT_B8G8R8A8_SRGB,
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
}

void Swapchain::create_frame(Swapchain::Frame& frame) {
  VkResult res_;

  {  // command pool
    VkCommandPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_queue_family.graphics.value().idx,
    };
    res_ = vkCreateCommandPool(m_device, &info, nullptr, &frame.pool);
    XEV_ASSERT_VK(res_, "Failed to create frame command pool");
  }

  {  // command buffer
    VkCommandBufferAllocateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_frames[i].cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    res_ = vkAllocateCommandBuffers(m_device, &info, &frame.render_cmdbuf);
    XEV_ASSERT_VK(res_, "Failed to allocate frame command buffer");
  }

  {  // fence for command buffer
    VkFenceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    res_ = vkCreateFence(m_device, &info, nullptr, &frame.render_fence);
    XEV_ASSERT_VK(res_, "Failed to create frame render fence");
  }

  {  // semaphores for render and present
    VkSemaphoreCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    res_ = vkCreateSemaphore(m_device, &info, nullptr, &frame.render_sem);
    XEV_ASSERT_VK(res_, "Failed to create frame drawn semaphore");
    res_ = vkCreateSemaphore(m_device, &info, nullptr, &frame.present_sem);
    XEV_ASSERT_VK(res_, "Failed to create frame image semaphore");
  }
}

void Swapchain::destroy_frame(Swapchain::Frame& frame) {
  vkDestroyFence(m_device, frame.render_fence, nullptr);
  vkDestroySemaphore(m_device, frame.render_sem, nullptr);
  vkDestroySemaphore(m_device, frame.present_sem, nullptr);
  vkDestroyCommandPool(m_device, frame.cmd_pool, nullptr);
}

VkCommandBuffer Swapchain::acquire_frame() {
  VkResult res_;
  const Swapchain::Frame& frame = m_frames[m_frame_idx];
  res_ = vkWaitForFences(m_device, 1, &frame.render_fence, VK_TRUE, NO_TIMEOUT);
  XEV_ASSERT_VK(res_, "Failed to acquire frame");

  VkCommandBufferBeginInfo info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  res_ = vkBeginCommandBuffer(frame.render_cmdbuf, &info);
  XEV_ASSERT_VK(res_, "Failed to begin command buffer");

  return frame.render_cmdbuf;
}

const Image& Swapchain::acquire_image(VkSemaphore swapchain_sem) {
  VkResult res_;
  uint32_t idx;
  res_ = vkAcquireNextImageKHR(m_device, m_swapchain, NO_TIMEOUT, swapchain_sem,
                               VK_NULL_HANDLE, &idx);
  // if (res_ == VK_ERROR_OUT_OF_DATE_KHR || res_ == VK_SUBOPTIMAL_KHR) {
  //   m_is_swapchain_dirty = true;
  XEV_ASSERT_VK(res_, "Failed to acquire swapchain image");
  return m_images[idx];
}

void Swapchain::release_frame() {
  VkResult res_;

  const Swapchain::Frame& frame = m_frames[m_frame_idx];
  const Image& image = acquire_image(frame.swapchain_sem);

  // reset fence
  XEV_ASSERT_VK(vkResetFences(m_device, 1, &current_frame.fence_render));

  VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

  // clearing
  layout = common::update_layout(cmd, swapchain_image, layout,
                                 VK_IMAGE_LAYOUT_GENERAL);

  VkImageSubresourceRange swapchain_range =
      common::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);
  VkClearColorValue clear_val = {{args.clear_color[0], args.clear_color[1],
                                  args.clear_color[2], args.clear_color[3]}};

  vkCmdClearColorImage(cmd, swapchain_image, VK_IMAGE_LAYOUT_GENERAL,
                       &clear_val, 1, &swapchain_range);

  // copy to swapchain
  // NOTE: image is already in TRANSFER_SRC_OPTIMAL — Renderer3D transitions it
  // at the end of draw(). Do NOT re-issue the barrier with a wrong oldLayout.
  if (args.copy_to_swapchain && image.image != VK_NULL_HANDLE) {
    // XEV_INFO("This is executed.");
    layout = common::update_layout(cmd, swapchain_image, layout,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    common::copy_image(cmd, image.image, swapchain_image,
                       {image.width, image.height}, m_extent, args.filter);
  }

  // present
  layout = common::update_layout(cmd, swapchain_image, layout,
                                 VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  XEV_ASSERT_VK(vkEndCommandBuffer(cmd));

  {  // submit
    VkSemaphoreSubmitInfo wait_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = frame.swapchain_sema,
        .value = 1,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
        .deviceIndex = 0,
    };
    VkSemaphoreSubmitInfo signal_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = frame.render_sema,
        .value = 1,
        .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
        .deviceIndex = 0,
    };

    VkCommandBufferSubmitInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cmd,
    };
    VkSubmitInfo2 info2 = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &wait_info,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &info,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &signal_info,
    };
    XEV_ASSERT_VK(
        vkQueueSubmit2(m_gfx_queue, 1, &info2, current_frame.fence_render));
  }

  {  // present
    VkPresentInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &current_frame.sem_render,
        .swapchainCount = 1,
        .pSwapchains = &m_swapchain,
        .pImageIndices = &img_idx,
    };

    res_ = vkQueuePresentKHR(m_gfx_queue, &info);
    if (res_ != VK_SUCCESS) {
      m_is_swapchain_dirty = true;
      if (res_ != VK_SUBOPTIMAL_KHR) {
        XEV_ERROR("Failed to present {}", (int)res_);
      }
    }
  }

  m_current_frame_idx = (m_current_frame_idx + 1) % NUM_FRAME_OVERLAP;
}

}  // namespace xev
