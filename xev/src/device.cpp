#include <xev/device.h>
#include <xev/logger.h>
#include <xev/volk.h>

namespace xev {

Device::Device() {
  init_instance();
  pick_physical_device();
  find_queue_family();
  init_logical_device();
}

Device::Device(SDL_Window* window) {
  init_instance();
  pick_physical_device();
  init_surface(window);
  find_queue_family();
  init_logical_device();
}

~Device() {
  if (device != VK_NULL_HANDLE)
    vkDestroyDevice(device, nullptr);
  if (surface != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(instance, surface, nullptr);
  if (instance != VK_NULL_HANDLE)
    vkDestroyInstance(instance, nullptr);
}

void Device::init_instance() {
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

  VkInstanceCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .flags = has_portability
                   ? (VkInstanceCreateFlags)
                         VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
                   : 0,
      .pApplicationInfo = &vkapp_info,
      .enabledExtensionCount = (uint32_t)extensions.size(),
      .ppEnabledExtensionNames = extensions.data(),
  };
  res_ = vkCreateInstance(&info, nullptr, &instance);
  if (res_ != VK_SUCCESS) {
    XEV_ERROR("Vulkan instance creation failed: {}", (int)res_);
    return;
  }

  volkLoadInstance(instance);
}

void Device::pick_physical_device() {
  VkResult res_;
  uint32_t num_physdev;
  vkEnumeratePhysicalDevices(instance, &num_physdev, nullptr);
  if (num_physdev < 1) {
    XEV_ERROR("No physical device found");
    return;
  }

  std::vector<VkPhysicalDevice> physdevs(num_physdev);
  res_ = vkEnumeratePhysicalDevices(instance, &num_physdev, physdevs.data());
  if (res_ != VK_SUCCESS) {
    XEV_ERROR("Failed to get physical device: {}", (int)res_);
    return;
  }
  physical_device = physdevs[0];

  VkPhysicalDeviceProperties dev_prop;
  vkGetPhysicalDeviceProperties(physical_device, &dev_prop);
  XEV_INFO("Using physical device {}", dev_prop.deviceName);
}

void Device::init_surface(SDLWindow* window) {
  if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
    XEV_ERROR("Failed to create surface: {}", SDL_GetError());
  }
}

void Device::find_queue_family() {
  uint32_t cnt;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &cnt, nullptr);

  std::vector<VkQueueFamilyProperties> props(cnt);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &cnt, props.data());

  for (uint32_t i = 0; i < cnt; i++) {
    if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      queue_family.graphics = {i, 1};
    if (props[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
      queue_family.compute = {i, 1};

    if (surface != VK_NULL_HANDLE) {
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface,
                                           &presentSupport);
      if (presentSupport)
        queue_family.present = {i, 1};
    }
  }
}

void Device::init_logical_device() {
  VkResult res_;

  uint32_t cnt;
  vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &cnt, nullptr);

  std::vector<VkExtensionProperties> exts(cnt);
  vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &cnt,
                                       exts.data());

  std::vector<const char*> ext_names;
  for (const auto& ext : exts) {
    if (std::string(ext.extensionName) == "VK_KHR_portability_subset") {
      ext_names.push_back("VK_KHR_portability_subset");
    }
  }
  ext_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  std::set<uint32_t> unique_family_idx = {queue_family.graphics.value().idx};
  if (surface != VK_NULL_HANDLE)
    unique_family_idx.insert(queue_family.present.value().idx);

  std::vector<VkDeviceQueueCreateInfo> queue_infos;
  float queue_priority = 1.0f;
  for (uint32_t family_idx : unique_family_idx) {
    VkDeviceQueueCreateInfo queue_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = family_idx,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };
    queue_infos.push_back(queue_info);
  }

  VkPhysicalDeviceVulkan13Features vk13feat = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .pNext = nullptr,
      .synchronization2 = VK_TRUE,
      .dynamicRendering = VK_TRUE,
  };
  VkPhysicalDeviceVulkan12Features vk12feat = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
      .pNext = &vk13feat,
      .descriptorIndexing = VK_TRUE,
      .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
      .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
      .descriptorBindingPartiallyBound = VK_TRUE,
      .descriptorBindingVariableDescriptorCount = VK_TRUE,
      .runtimeDescriptorArray = VK_TRUE,
      .scalarBlockLayout = VK_TRUE,
      .bufferDeviceAddress = VK_TRUE,
  };
  VkPhysicalDeviceFeatures2 m_feat = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = &vk12feat,
      .features =
          {
              .imageCubeArray = VK_TRUE,
              .geometryShader = VK_FALSE,
              .depthClamp = VK_TRUE,
              .samplerAnisotropy = VK_TRUE,
          },
  };
  VkDeviceCreateInfo info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &m_feat,
      .queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size()),
      .pQueueCreateInfos = queue_infos.data(),
      .enabledExtensionCount = static_cast<uint32_t>(ext_names.size()),
      .ppEnabledExtensionNames = ext_names.data(),
  };

  res_ = vkCreateDevice(physical_device, &info, nullptr, &logical_device);
  if (res_ != VK_SUCCESS) {
    XEV_ERROR("Failed to create logical device: {}", (int)res_);
    return;
  }

  volkLoadDevice(logical_device);
}

}  // namespace xev
