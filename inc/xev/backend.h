#pragma once
#include <SDL3/SDL.h>
#include <optional>
#include <string>
#include <vector>
#include <volk.h>
#include <xev/logger.h>

namespace xev {

enum QFAM {
  Q_GRAPHICS,
  Q_PRESENT,
  Q_COMPUTE,
};

struct QueueFamilyEntry {
  uint32_t idx;
  uint32_t cnt; // number of queues within the family
};

struct QueueFamily {
  std::optional<QueueFamilyEntry> gfx;
  std::optional<QueueFamilyEntry> pre;
  std::optional<QueueFamilyEntry> com;

  bool isComplete() { return gfx.has_value() && pre.has_value(); }
};

typedef uint32_t BufferHandle;
constexpr BufferHandle BUFFER_NULL_HANDLE = 0;

class Buffer {
public:
  Buffer(std::string_view name, VkDeviceSize size, VkBufferUsageFlags usage,
         VkMemoryPropertyFlags properties);
  bool is_valid() { return m_valid; }
  std::string name = "";

private:
  bool m_valid = false;
  VkBuffer m_buffer = VK_NULL_HANDLE;
  VkBufferMemory m_memory = VK_NULL_HANDLE;
};

class Backend {
public:
  Backend(SDL_Window *window);
  ~Backend();

  VkDevice get_device() { return m_dev; }
  VkPhysicalDevice get_physical_device() { return m_physical_dev; }
  VkQueue retrieve_queue(QFAM qfam);
  VkQueue retrieve_queue(QFAM qfam, uint32_t qidx);
  std::optional<QueueFamilyEntry> get_queue_family_index(QFAM qfam);
  VkSurfaceFormatKHR get_format() { return m_ideal_format; }

  VkSwapchainKHR get_swapchain() { return m_swapchain; }
  VkExtent2D get_extent() { return m_extent; }
  std::vector<VkImage> get_swapchain_images() { return m_swapchain_images; }
  std::vector<VkImageView> get_swapchain_image_views() {
    return m_swapchain_image_views;
  }

  const char *app_name = "xev_app";
  uint32_t app_version = VK_MAKE_VERSION(0, 0, 0);
  const char *engine_name = "xev_engine";
  uint32_t engine_version = VK_MAKE_VERSION(0, 0, 0);

  void create_buffer(VkDevice size, VkBufferUsageFlags usage,
                     VkMemoryPropertyFlags properties);

private:
  QueueFamily getQueueFamily(VkPhysicalDevice device, VkSurfaceKHR surface);
  VkSwapchainKHR create_swapchain(VkDevice device, VkSurfaceKHR surface);
  VkSwapchainKHR recreate_swapchain(VkDevice device, VkSurfaceKHR surface);

  VkSurfaceFormatKHR m_ideal_format = {.format = VK_FORMAT_B8G8R8A8_SRGB,
                                       .colorSpace =
                                           VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

  VkInstance m_inst = VK_NULL_HANDLE;
  VkSurfaceKHR m_surf = VK_NULL_HANDLE;
  VkPhysicalDevice m_physical_dev = VK_NULL_HANDLE;
  VkDevice m_dev = VK_NULL_HANDLE;
  VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
  VkExtent2D m_extent = {0, 0};
  std::vector<VkImage> m_swapchain_images;
  std::vector<VkImageView> m_swapchain_image_views;

  QueueFamily m_qfam;
  std::vector<const char *> m_ext;

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

  VkQueue m_gfx_queue = VK_NULL_HANDLE;
  VkQueue m_pre_queue = VK_NULL_HANDLE;

  std::unordered_map<BufferHandle, std::unique_ptr<Buffer>> m_buffers;
};

} // namespace xev
