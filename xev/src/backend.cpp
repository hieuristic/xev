#include <xev/backend.h>
#include <xev/commmon.h>

namespace xev {

Backend::Backend(SDL_Window* window) {
  VkResult res_;

  res_ = volkInitialize();
  if (res_ != VK_SUCCESS) {
    XEV_ERROR("VOLK init failed");
  }

  uint32_t vkapi_version;
  vkEnumerateInstanceVersion(&vkapi_version);
  XEV_DEBUG("API Version: {}.{}.{}", VK_API_VERSION_MAJOR(vkapi_version),
            VK_API_VERSION_MINOR(vkapi_version),
            VK_API_VERSION_PATCH(vkapi_version));

  VkApplicationInfo vkapp_info{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = app_name,
      .applicationVersion = app_version,
      .pEngineName = engine_name,
      .engineVersion = engine_version,
      .apiVersion = vkapi_version,
  };

  uint32_t extension_count = 0;
  char const* const* sdl_extensions =
      SDL_Vulkan_GetInstanceExtensions(&extension_count);
  std::vector<const char*> extensions(sdl_extensions,
                                      sdl_extensions + extension_count);

  uint32_t instance_ext_count = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &instance_ext_count, nullptr);
  std::vector<VkExtensionProperties> instance_exts(instance_ext_count);
  vkEnumerateInstanceExtensionProperties(nullptr, &instance_ext_count,
                                         instance_exts.data());

  bool has_portability = false;
  for (const auto& ext : instance_exts) {
    if (std::string(ext.extensionName) == "VK_KHR_portability_enumeration") {
      has_portability = true;
      extensions.push_back("VK_KHR_portability_enumeration");
      break;
    }
  }

  VkInstanceCreateInfo vkcreate_info = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .flags = has_portability
                   ? (VkInstanceCreateFlags)
                         VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
                   : 0,
      .pApplicationInfo = &vkapp_info,
      .enabledExtensionCount = (uint32_t)extensions.size(),
      .ppEnabledExtensionNames = extensions.data(),
  };
  res_ = vkCreateInstance(&vkcreate_info, nullptr, &m_instance);
  if (res_ != VK_SUCCESS) {
    XEV_ERROR("Vulkan instance creation failed: {}", (int)res_);
    return;
  }

  volkLoadInstance(m_instance);

  if (!SDL_Vulkan_CreateSurface(window, m_instance, nullptr, &m_surface)) {
    XEV_ERROR("Failed to create surface: {}", SDL_GetError());
    return;
  }

  uint32_t num_physdev;
  vkEnumeratePhysicalDevices(m_instance, &num_physdev, nullptr);
  if (num_physdev < 1) {
    XEV_ERROR("No physical device found");
    return;
  }

  std::vector<VkPhysicalDevice> physdevs(num_physdev);
  res_ = vkEnumeratePhysicalDevices(m_instance, &num_physdev, physdevs.data());
  if (res_ != VK_SUCCESS) {
    XEV_ERROR("Failed to get physical device: {}", (int)res_);
    return;
  }
  m_physical_device = physdevs[0];

  VkPhysicalDeviceProperties dev_prop;
  vkGetPhysicalDeviceProperties(m_physical_device, &dev_prop);
  XEV_INFO("Using physical device {}", dev_prop.deviceName);

  m_qfam = get_queue_family(m_physical_device, m_surface);

  uint32_t device_ext_count;
  vkEnumerateDeviceExtensionProperties(m_physical_device, nullptr,
                                       &device_ext_count, nullptr);
  std::vector<VkExtensionProperties> device_exts(device_ext_count);
  vkEnumerateDeviceExtensionProperties(m_physical_device, nullptr,
                                       &device_ext_count, device_exts.data());

  for (const auto& ext : device_exts) {
    if (std::string(ext.extensionName) == "VK_KHR_portability_subset") {
      m_ext.push_back("VK_KHR_portability_subset");
    }
  }
  m_ext.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  std::set<uint32_t> uniqueQueueFamilies = {m_qfam.gfx.value().idx,
                                            m_qfam.pre.value().idx};
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  float queue_priority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkDeviceCreateInfo dev_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &m_feat,
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
      .pQueueCreateInfos = queueCreateInfos.data(),
      .enabledExtensionCount = static_cast<uint32_t>(m_ext.size()),
      .ppEnabledExtensionNames = m_ext.data(),
  };

  res_ = vkCreateDevice(m_physical_device, &dev_info, nullptr, &m_device);
  if (res_ != VK_SUCCESS) {
    XEV_ERROR("Failed to create logical device: {}", (int)res_);
    return;
  }

  volkLoadDevice(m_device);
  m_allocator =
      create_memory_allocator(m_instance, m_physical_device, m_device);

  m_gfx_queue = retrieve_queue(Q_GRAPHICS);
  m_pre_queue = retrieve_queue(Q_PRESENT);

  m_swapchain = create_swapchain(m_device, m_surface);

  // setup bindless descripter set
  {  // descriptor pool
    std::array<VkDescriptorPoolSize, 2> sizes = {
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_BINDLESS_TEXTURE},
        {VK_DESCRIPTOR_TYPE_SAMPLER, MAX_BINDLESS_SAMPLER},
    };
    VkDescriptorPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
        .maxSets = MAX_BINDLESS_TEXTURE * 2,
        .poolSizeCount = 2,
        .pPoolSizes = sizes.data(),
    };

    res_ = vkCreateDescriptorPool(m_device, &info, nullptr, &m_desc_pool);
    XEV_ASSERT_VK(res_, "Failed to create descriptor pool");
  }

  {  // creating descriptor set layout
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
        {
            .binding = TEXTURE_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = MAX_BINDLESS_TEXTURE,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
        {
            .binding = SAMPLER_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = MAX_BINDLESS_SAMPLER,
            .stageFlags = VK_SHADER_STAGE_ALL,
        }};
    std::array<VkDescriptorBindingFlags, 2> flags = {
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
    };

    VkDescriptorSetLayoutBindingFlagsCreateInfo flag_info = {
        .sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = 2,
        .pBindingFlags = flags.data(),
    };

    VkDescriptorSetLayoutCreateInfo info =
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &flag_info,
            .flags =
                VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
            .bindingCount = 2,
            .pBindings = bindings.data(),
        }

    res_ = vkCreateDescriptorSetLayout(m_device, &info, nullptr,
                                       &m_desc_set_layout);
    XEV_ASSERT_VK(res_, "Failed to create descriptor set layout");
  }

  {  // creating descriptor set
    VkDescriptorSetAllocateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_desc_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_desc_set_layout,
    };

    // uint32_t max_binding = MAX_BINDLESS_TEXTURE - 1;
    res_ = vkAllocateDescriptorSets(m_device, &info, &m_desc_set);
    XEV_ASSERT_VK(res_, "Failed to create descriptor set.");
  }

  {  // setup sampler
    VkSamplerCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = MAX_SAMPLER_ANISOTROPY,
    };
    res_ = vkCreateSampler(m_device, &info, nullptr, &m_linear_sampler);
    create_sampler(LINEAR_SAMPLER_ID, m_linear_sampler);
  }
}

Backend::~Backend() {
  if (m_allocator != nullptr)
    vmaDestroyAllocator(m_allocator);

  vkDestroySampler(m_device, m_linear_sampler, nullptr);
  vkDestroyDescriptorSetLayout(m_device, m_desc_set_layout, nullptr);
  vkDestroyDescriptorPool(m_device, m_desc_pool, nullptr);

  for (auto imageView : m_swapchain_image_views) {
    vkDestroyImageView(m_device, imageView, nullptr);
  }
  if (m_swapchain != VK_NULL_HANDLE)
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
  if (m_device != VK_NULL_HANDLE)
    vkDestroyDevice(m_device, nullptr);
  if (m_surface != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
  if (m_instance != VK_NULL_HANDLE)
    vkDestroyInstance(m_instance, nullptr);
}

QueueFamily Backend::get_queue_family(VkPhysicalDevice device,
                                      VkSurfaceKHR surface) {
  QueueFamily qfam;
  uint32_t qfam_cnt;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &qfam_cnt, nullptr);

  std::vector<VkQueueFamilyProperties> qfamprops(qfam_cnt);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &qfam_cnt, qfamprops.data());

  for (uint32_t i = 0; i < qfam_cnt; i++) {
    if (qfamprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      qfam.gfx = {i, 1};
    if (qfamprops[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
      qfam.com = {i, 1};

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
    if (presentSupport)
      qfam.pre = {i, 1};
  }
  return qfam;
}

std::optional<QueueFamilyEntry> Backend::get_queue_family_index(QFAM qfam) {
  switch (qfam) {
    case Q_GRAPHICS:
      return m_qfam.gfx;
    case Q_PRESENT:
      return m_qfam.pre;
    case Q_COMPUTE:
      return m_qfam.com;
  }
  return std::nullopt;
}

VkQueue Backend::retrieve_queue(QFAM qfam) {
  return retrieve_queue(qfam, 0);
}

VkQueue Backend::retrieve_queue(QFAM qfam, uint32_t qidx) {
  VkQueue queue = VK_NULL_HANDLE;
  if (m_device == VK_NULL_HANDLE) {
    XEV_ERROR("Device not initialized");
    return queue;
  }

  std::optional<QueueFamilyEntry> qfam_entry = get_queue_family_index(qfam);
  if (!qfam_entry.has_value()) {
    XEV_ERROR("No queue within the given queue family");
    return queue;
  }
  if (qidx >= qfam_entry.value().cnt) {
    XEV_ERROR("Invalid queue index (out of range)");
    return queue;
  }

  VkDeviceQueueInfo2 q_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,
      .queueFamilyIndex = qfam_entry.value().idx,
      .queueIndex = qidx,
  };

  vkGetDeviceQueue2(m_device, &q_info, &queue);
  return queue;
}

VkSwapchainKHR Backend::create_swapchain(VkDevice device,
                                         VkSurfaceKHR surface) {
  VkResult res_;
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;

  if (!m_qfam.isComplete()) {
    XEV_ERROR("Queue families incomplete for swapchain");
    return swapchain;
  }

  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, surface,
                                            &capabilities);

  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, surface,
                                       &format_count, nullptr);
  if (format_count == 0) {
    XEV_ERROR("No surface formats supported");
    return swapchain;
  }
  std::vector<VkSurfaceFormatKHR> formats(format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, surface,
                                       &format_count, formats.data());

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, surface,
                                            &present_mode_count, nullptr);
  if (present_mode_count == 0) {
    XEV_ERROR("No surface present modes supported");
    return swapchain;
  }
  std::vector<VkPresentModeKHR> present_modes(present_mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      m_physical_device, surface, &present_mode_count, present_modes.data());

  VkSurfaceFormatKHR surface_format = formats[0];
  for (const auto& format : formats) {
    if (format.format == m_ideal_format.format &&
        format.colorSpace == m_ideal_format.colorSpace) {
      surface_format = format;
      break;
    }
  }

  if (surface_format.format != m_ideal_format.format ||
      surface_format.colorSpace != m_ideal_format.colorSpace) {
    XEV_ERROR("Ideal format not supported by swapchain");
    return swapchain;
  }

  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
  for (const auto& mode : present_modes) {
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      present_mode = mode;
      break;
    }
  }

  VkExtent2D extent = capabilities.currentExtent;

  uint32_t image_count = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 &&
      image_count > capabilities.maxImageCount) {
    image_count = capabilities.maxImageCount;
  }

  XEV_INFO("image count supported for swapchain {}",
           capabilities.minImageCount);

  VkSwapchainCreateInfoKHR swapchain_info{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = surface,
      .minImageCount = image_count,
      .imageFormat = surface_format.format,
      .imageColorSpace = surface_format.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .preTransform = capabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = present_mode,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE,
  };

  uint32_t queue_family_indices[] = {m_qfam.gfx.value().idx,
                                     m_qfam.pre.value().idx};
  if (m_qfam.gfx.value().idx != m_qfam.pre.value().idx) {
    swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_info.queueFamilyIndexCount = 2;
    swapchain_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  res_ = vkCreateSwapchainKHR(device, &swapchain_info, nullptr, &m_swapchain);
  if (res_ != VK_SUCCESS) {
    XEV_ERROR("Swapchain creation failed: {}", (int)res_);
    return VK_NULL_HANDLE;
  }

  m_extent = extent;

  uint32_t real_image_count;
  vkGetSwapchainImagesKHR(device, m_swapchain, &real_image_count, nullptr);
  m_swapchain_images.resize(real_image_count);
  vkGetSwapchainImagesKHR(device, m_swapchain, &real_image_count,
                          m_swapchain_images.data());

  m_swapchain_image_views.resize(real_image_count);
  for (uint32_t i = 0; i < real_image_count; i++) {
    VkImageViewCreateInfo view_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_swapchain_images[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = surface_format.format,
        .components = {VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange =
            {
                .aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
    res_ = vkCreateImageView(device, &view_info, nullptr,
                             &m_swapchain_image_views[i]);
    XEV_ERROR_IF_FAILED(res_, "Failed to create swapchain image view!");
  }

  return m_swapchain;
}

VkSwapchainKHR Backend::recreate_swapchain(VkDevice device,
                                           VkSurfaceKHR surface) {
  vkDeviceWaitIdle(device);
  for (auto imageView : m_swapchain_image_views) {
    vkDestroyImageView(device, imageView, nullptr);
  }
  m_swapchain_image_views.clear();
  if (m_swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device, m_swapchain, nullptr);
  }
  return create_swapchain(device, surface);
}

Buffer Backend::create_buffer(std::string_view name,
                              VkDeviceSize size,
                              VkBufferUsageFlags usage,
                              VmaMemoryUsage mem_usage) const {
  Buffer buffer{};

  VkBufferCreateInfo buffer_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = usage,
  };

  VmaAllocationCreateInfo alloc_info = {
      .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
               VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
      .usage = mem_usage,
  };

  VkResult res_ =
      vmaCreateBuffer(m_allocator, &buffer_info, &alloc_info, &buffer.buffer,
                      &buffer.alloc, &buffer.alloc_info);
  XEV_ASSERT(res_ == VK_SUCCESS);

  XEV_INFO("Created buffer '{}' ({} bytes)", name, size);
  return buffer;
}

void Backend::destroy_buffer(Buffer buffer) {
  vmaDestroyBuffer(m_allocator, buffer.buffer, buffer.alloc);
}

VmaAllocator Backend::create_memory_allocator(VkInstance instance,
                                              VkPhysicalDevice physical_device,
                                              VkDevice device) {
  VkResult res_;
  VmaAllocator allocator;

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

  res_ = vmaCreateAllocator(&info, &allocator);
  XEV_ASSERT(res_ == VK_SUCCESS);

  return allocator;
}

VkCommandBuffer enter_frame() {
  const Frame& frame = m_frames[m_current_frame_idx];
  XEV_ASSERT_VK(
      vkWaitForFences(m_device, 1, &frame.fence_render, VK_TRUE, NO_TIMEOUT));

  const VkCommandBuffer& cmd = frame.cmd_buffer;
  VkCommandBufferBeginInfo begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  XEV_ASSERT_VK(vkBeginCommandBuffer(cmd, &begin_info));

  return cmd;
}

void leave_frame(VkCommandBuffer cmd,
                 const Image& image,
                 const FrameArg& args) {
  VkResult res_;

  // acquire swapchain image
  uint32_t img_idx;
  const VkImage& swapchain_image;
  const Frame& current_frame = m_frames[m_current_frame_idx];
  res_ = vkAcquireNextImageKHR(m_device, m_swapchain, NO_TIMEOUT,
                               current_frame.sem_swapchain, VK_NULL_HANDLE,
                               &img_idx);
  if (res_ == VK_ERROR_OUT_OF_DATE_KHR || res_ == VK_SUBOPTIMAL_KHR) {
    m_is_swapchain_dirty = true;
    swapchain_image = m_images[img_idx];
  } else {
    XEV_ASSERT_VK(res_, "Failed to acquaire swapchain image");
    return;
  }

  // reset fence
  XEV_ASSERT_VK(vkResetFences(device, 1, &current_frame));

  VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

  // clearing
  layout = common::update_layout(cmd, swapchain_image, layout,
                                 VK_IMAGE_LAYOUT_GENERAL);
  vkCmdClearColorImage(
      cmd, swapchain_image, VK_IMAGE_LAYOUT_GENERAL,
      &common::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT), 1,
      &{{args.clear_color}});

  // copy to swapchain
  if (args.copy_to_swapchain) {
    common::update_layout(cmd, image.image, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    layout = common::update_layout(cmd, swapchain_image, layout,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    common::copy_image(cmd, image.image, swapchain_image, image.ext,
                       m_swapchain_ext, args.filter);
  }

  // present
  update_layout(cmd, swapchain_image, layout, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  XEV_ASSERT_VK(vkEndCommandBuffer(cmd));

  {  // submit
    VkSemaphoreSubmitInfo wait_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = current_frame.sem_swapchain,
        .value = 1,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
        .deviceIndex = 0,
    };
    VkSemaphoreSubmitInfo wait_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = current_frame.sem_swapchain,
        .value = 1,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
        .deviceIndex = 0,
    };
    const auto signalInfo = vkinit::semaphoreSubmitInfo(
        VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, current_frame.sem_render);

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
        XEV_ERROR("Failed to present {}", res_);
      }
    }
  }

  m_current_frame_idx = (m_current_frame_idx + 1) % NUM_FRAME_OVERLAP;
}

VkPipelineShaderStageCreateInfo Backend::create_shader_stage(
    const char* path,
    VkShaderStageFlagBits stage_bit) {
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    XEV_ERROR("Failed to read {}", path);
  }

  std::vector<char> m_shader_src;
  auto size = file.tellg();
  m_shader_src.resize(size);
  file.seekg(0, std::ios::beg);
  file.read(m_shader_src.data(), static_cast<std::streamsize>(size));
  file.close();

  VkResult res_;
  VkShaderModule module{VK_NULL_HANDLE};

  VkShaderModuleCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = size,
      .pCode = reinterpret_cast<const uint32_t*>(m_shader_src.data()),
  };

  res_ = vkCreateShaderModule(m_device, &info, nullptr, &module);
  VK_ASSERT_VK(res_, "Failed to load shader module");

  return VkPipelineShaderStageCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = stage_bit,
      .module = module,
      .pName = "main",
  };
}

VkPipelineLayout Backend::create_pipeline_layout(
    std::span<const VkDescriptorSetLayout> layouts,
    std::span<const VkPushConstantRange> ranges) {
  VkResult res_;
  VkPipelineLayout layout;

  VkPipelineCreateInfo info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = static_cast<uint32_t>(layouts.size()),
      .pSetLayouts = layouts.data(),
      .pushConstantRangeCount = static_cast<uint32_t>(ranges.size()),
      .pPushConstantRanges = ranges.data(),
  };

  res_ = vkCreatePipelineLayout(m_device, &info, nullptr & layout);
  XEV_ASSERT_VK(res_, "Failed to create pipeline layout.");
  return layout;
}

void Backend::create_sampler(uint32_t id, VkSampler sampler) {
  VkDescriptorImageInfo img_info = {
      .sampler = sampler,
      .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
  };
  VkWriteDescriptorSet write_set = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = m_desc_set,
      .dstBinding = SAMPLER_BINDING,
      .dstArrayElement = id,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
      .pImageInfo = &img_info,
  };
  vkUpdateDescriptorSets(m_device, 1, &write_set, 0, nullptr);
}

VkPipeline Backend::create_pipeline(const PipelineInfo& info) {
  VkPipelineViewportStateCreateInfo viewport_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1,
  };

  VkPipelineColorBlendAttachmentState blend_attachments = info.enable_blend
    ? {
        .blendEnable = VK_TRUE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .srcColorBlendFactor = info.blend_src_factor,
        .dstColorBlendFactor = info.blend_dst_factor,
        .colorBlendOp = info.blend_op,
        .srcAlphaBlendFactor = info.blend_src_alpha_factor,
        .dstAlphaBlendFactor = info.blend_dst_alpha_factor,
        .alphaBlendOp = info.blend_op,
      }
    : {
          .blendEnable = VK_FALSE,
          .colorWriteMask =
              VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };

  VkPipelineColorBlendStateCreateInfo blend_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &blend_attachments,
  };

  VkPipelineVertexInputStateCreateInfo vertex_input_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
  };
  auto dynamic_states =
      std::vector{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR}

  VkPipelineDynamicStateCreateInfo dynamic_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = (std::uint32_t)dynamic_tates.size(),
      .pDynamicStates = dynamic_tates.data(),
  };

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = info.topology,
      .primitiveRestartEnable = VK_FALSE,
  };
  VkPipelineRasterizationStateCreateInfo rasterizer_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .polygonMode = info.polygon,
      .lineWidth = 1.f,
  };
  VkPipelineMultisampleStateCreateInfo multisampling_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .sampleShadingEnable = VK_FALSE,
      .rasterizationSamples = info.samples,
      .minSampleShading = 1.0f,
      .alphaToCoverageEnable = VK_FALSE,
      .alphaToOneEnable = VK_FALSE,
  };
  VkPipelineDepthStencilStateCreateInfo depth_stencil_state =
      info.enable_depth ? {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthTestEnable = VK_TRUE;
    .depthWriteEnable = VK_TRUE;
    .depthCompareOp = info.depth_op;
    .depthBoundsTestEnable = VK_FALSE;
    .stencilTestEnable = VK_FALSE;
    .front = {};
    .back = {};
    .minDepthBounds = 0.f;
    .maxDepthBounds = 1.f;
  } : {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthTestEnable = VK_FALSE;
    .depthWriteEnable = VK_FALSE;
    .depthCompareOp = VK_COMPARE_OP_NEVER;
    .depthBoundsTestEnable = VK_FALSE;
    .stencilTestEnable = VK_FALSE;
    .front = {};
    .back = {};
    .minDepthBounds = 0.f;
    .maxDepthBounds = 1.f;
  };

  VkPipelineRenderingCreateInfo render_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &info.color_format,
      .depthAttachmentFormat = info.depth_format,
  };

  std::array<&VkPipelineShaderStageCreateInfo, 2> shader_stages = {
      info.vert_shader,
      info.frag_shader,
  };

  VkGraphicsPipelineCreateInfo pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = &render_info,
      .stageCount = (std::uint32_t)shader_stages.size(),
      .pStages = shader_stages.data(),
      .pVertexInputState = &vertex_input_state,
      .pInputAssemblyState = &input_assembly_state,
      .pViewportState = &viewport_state,
      .pRasterizationState = &rasterizer_state,
      .pMultisampleState = &multisampling_state,
      .pDepthStencilState = &depth_stencil_state,
      .pColorBlendState = &blend_state,
      .pDynamicState = &dynamic_info,
      .layout = &info.layout,
  };
}

}  // namespace xev
