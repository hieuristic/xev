#define VOLK_IMPLEMENTATION
#include <SDL3/SDL_vulkan.h>
#include <algorithm>
#include <set>
#include <string>
#include <vector>
#include <xev/backend.h>

namespace xev {

Backend::Backend(SDL_Window *window) {
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
  char const *const *sdl_extensions =
      SDL_Vulkan_GetInstanceExtensions(&extension_count);
  std::vector<const char *> extensions(sdl_extensions,
                                       sdl_extensions + extension_count);

  uint32_t instance_ext_count = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &instance_ext_count, nullptr);
  std::vector<VkExtensionProperties> instance_exts(instance_ext_count);
  vkEnumerateInstanceExtensionProperties(nullptr, &instance_ext_count,
                                         instance_exts.data());

  bool has_portability = false;
  for (const auto &ext : instance_exts) {
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
  res_ = vkCreateInstance(&vkcreate_info, nullptr, &m_inst);
  if (res_ != VK_SUCCESS) {
    XEV_ERROR("Vulkan instance creation failed: {}", (int)res_);
    return;
  }

  volkLoadInstance(m_inst);

  if (!SDL_Vulkan_CreateSurface(window, m_inst, nullptr, &m_surf)) {
    XEV_ERROR("Failed to create surface: {}", SDL_GetError());
    return;
  }

  uint32_t num_physdev;
  vkEnumeratePhysicalDevices(m_inst, &num_physdev, nullptr);
  if (num_physdev < 1) {
    XEV_ERROR("No physical device found");
    return;
  }

  std::vector<VkPhysicalDevice> physdevs(num_physdev);
  res_ = vkEnumeratePhysicalDevices(m_inst, &num_physdev, physdevs.data());
  if (res_ != VK_SUCCESS) {
    XEV_ERROR("Failed to get physical device: {}", (int)res_);
    return;
  }
  m_physical_dev = physdevs[0];

  VkPhysicalDeviceProperties dev_prop;
  vkGetPhysicalDeviceProperties(m_physical_dev, &dev_prop);
  XEV_INFO("Using physical device {}", dev_prop.deviceName);

  m_qfam = getQueueFamily(m_physical_dev, m_surf);

  uint32_t device_ext_count;
  vkEnumerateDeviceExtensionProperties(m_physical_dev, nullptr,
                                       &device_ext_count, nullptr);
  std::vector<VkExtensionProperties> device_exts(device_ext_count);
  vkEnumerateDeviceExtensionProperties(m_physical_dev, nullptr,
                                       &device_ext_count, device_exts.data());

  for (const auto &ext : device_exts) {
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

  res_ = vkCreateDevice(m_physical_dev, &dev_info, nullptr, &m_dev);
  if (res_ != VK_SUCCESS) {
    XEV_ERROR("Failed to create logical device: {}", (int)res_);
    return;
  }

  volkLoadDevice(m_dev);

  m_gfx_queue = retrieve_queue(Q_GRAPHICS);
  m_pre_queue = retrieve_queue(Q_PRESENT);

  create_swapchain(m_dev, m_surf);
}

Backend::~Backend() {
  for (auto imageView : m_swapchain_image_views) {
    vkDestroyImageView(m_dev, imageView, nullptr);
  }
  if (m_swapchain != VK_NULL_HANDLE)
    vkDestroySwapchainKHR(m_dev, m_swapchain, nullptr);
  if (m_dev != VK_NULL_HANDLE)
    vkDestroyDevice(m_dev, nullptr);
  if (m_surf != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(m_inst, m_surf, nullptr);
  if (m_inst != VK_NULL_HANDLE)
    vkDestroyInstance(m_inst, nullptr);
}

QueueFamily Backend::getQueueFamily(VkPhysicalDevice device,
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

VkQueue Backend::retrieve_queue(QFAM qfam) { return retrieve_queue(qfam, 0); }

VkQueue Backend::retrieve_queue(QFAM qfam, uint32_t qidx) {
  VkQueue queue = VK_NULL_HANDLE;
  if (m_dev == VK_NULL_HANDLE) {
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

  vkGetDeviceQueue2(m_dev, &q_info, &queue);
  return queue;
}

VkSwapchainKHR Backend::create_swapchain(VkDevice device,
                                         VkSurfaceKHR surface) {
  VkSuccess res_;
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;

  if (!m_qfam.isComplete()) {
    XEV_ERROR("Queue families incomplete for swapchain");
    return swapchain;
  }

  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_dev, surface,
                                            &capabilities);

  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_dev, surface, &format_count,
                                       nullptr);
  if (format_count == 0) {
    XEV_ERROR("No surface formats supported");
    return swapchain;
  }
  std::vector<VkSurfaceFormatKHR> formats(format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_dev, surface, &format_count,
                                       formats.data());

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_dev, surface,
                                            &present_mode_count, nullptr);
  if (present_mode_count == 0) {
    XEV_ERROR("No surface present modes supported");
    return swapchain;
  }
  std::vector<VkPresentModeKHR> present_modes(present_mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      m_physical_dev, surface, &present_mode_count, present_modes.data());

  VkSurfaceFormatKHR surface_format = formats[0];
  for (const auto &format : formats) {
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
  for (const auto &mode : present_modes) {
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
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
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

void Backend::create_buffer(std::string_view name, VkDeviceSize size,
                            VkBufferUsageFlags usage,
                            VkMemoryPropertyFlags properties) {
  std::unique_ptr<Buffer> buffer =
      std::make_unique<Buffer>(name, size, usage, properties);
  if (buffer.is_valid()) {
    m_buffers.push_back(std::move(buffer));
  }
}

Buffer::Buffer(std::string_view name, VkDeviceSize size,
               VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
  VkResult res_;

  VkBufferCreateInfo buffer_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };

  res_ = vkCreateBuffer(m_device, &buffer_info, nullptr, &buffer.buffer);
  XEV_ERROR_IF_FAILED(res_, "Failed to create buffer: {}", name);

  VkMemoryRequirements mem_req;
  vkGetBufferMemoryRequirements(m_device, buffer.buffer, &mem_req);

  VkMemoryAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = mem_req.size,
      .memoryTypeIndex = find_memory_type(mem_req.memoryTypeBits, properties),
  };

  res_ = vkAllocateMemory(device, &alloc_info, nullptr, &buffer.memory);
  XEV_ERROR_IF_FAILED(res_, "Failed to allocate buffer memory: {}", name);

  res_ = vkBindBufferMemory(device, buffer.buffer, buffer.memory, 0);
  if m_buffers
    .push_back(buffer);
}

Buffer::~Buffer() {
}

} // namespace xev
