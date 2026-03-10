#pragma once
#include <SDL3/SDL.h>
#include <xev/buffer.h>
#include <xev/logger.h>
#include <xev/vma.h>
#include <xev/volk.h>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace xev {

inline constexpr uint32_t NUM_FRAME_OVERLAP = 2;
inline constexpr uint64_t NO_TIMEOUT =
    std::numeric_limits<std::uint64_t>::max();

enum QFAM {
  Q_GRAPHICS,
  Q_PRESENT,
  Q_COMPUTE,
};

struct QueueFamilyEntry {
  uint32_t idx;
  uint32_t cnt;  // number of queues within the family
};

struct QueueFamily {
  std::optional<QueueFamilyEntry> gfx;
  std::optional<QueueFamilyEntry> pre;
  std::optional<QueueFamilyEntry> com;

  bool isComplete() { return gfx.has_value() && pre.has_value(); }
};

struct Frame {
  VkCommandPool cmd_pool;
  VkCommandBuffer cmd_buffer;
  VkSemaphore sem_swapchain;
  VkSemaphore sem_render;
  VkFence fence_render;
};

/*
This class handle most of the VulkanAPI calls. It is responsible for
creating/destroying buffers, images, command buffers, as well as managing
and synchronizing the swapchain. All objects created with public methods
are NOT owned by the backend and their lifetime are handled externally.
*/
class Backend {
 public:
  Backend(SDL_Window* window);
  ~Backend();

  VkDevice get_device() { return m_device; }
  VkPhysicalDevice get_physical_device() { return m_physical_device; }
  VkSurfaceFormatKHR get_format() { return m_ideal_format; }
  VmaAllocator get_allocator() { return m_allocator; }

  VkSwapchainKHR get_swapchain() { return m_swapchain; }
  VkExtent2D get_extent() { return m_extent; }
  std::vector<VkImage> get_swapchain_images() { return m_swapchain_images; }
  std::vector<VkImageView> get_swapchain_image_views() {
    return m_swapchain_image_views;
  }

  const char* app_name = "xev_app";
  uint32_t app_version = VK_MAKE_VERSION(0, 0, 0);
  const char* engine_name = "xev_engine";
  uint32_t engine_version = VK_MAKE_VERSION(0, 0, 0);

  Buffer create_buffer(std::string_view name,
                       VkDeviceSize size,
                       VkBufferUsageFlags usage,
                       VmaMemoryUsage mem_usage = VMA_MEMORY_USAGE_AUTO) const;
  void destroy_buffer(Buffer);

  VkCommandBuffer enter_frame();
  void leave_frame();

 private:  // device and memmory allocator
  VkInstance m_instance = VK_NULL_HANDLE;
  VkSurfaceKHR m_surface = VK_NULL_HANDLE;
  VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
  VkDevice m_device = VK_NULL_HANDLE;
  VmaAllocator m_allocator = nullptr;

  VmaAllocator create_memory_allocator(VkInstance instance,
                                       VkPhysicalDevice physical_device,
                                       VkDevice device);

 private:  // swapchain and frames
  VkSurfaceFormatKHR m_ideal_format = {
      .format = VK_FORMAT_B8G8R8A8_SRGB,
      .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
  };
  VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
  bool m_is_swapchain_dirty = false;
  VkExtent2D m_extent = {0, 0};
  std::array<VkImage, NUM_FRAME_OVERLAP> m_images;
  std::array<Frame, NUM_FRAME_OVERLAP> m_frames;
  uint32_t m_current_frame_idx;
  std::vector<VkImage> m_swapchain_images;
  std::vector<VkImageView> m_swapchain_image_views;

  VkSwapchainKHR create_swapchain(VkDevice device, VkSurfaceKHR surface);
  VkSwapchainKHR recreate_swapchain(VkDevice device, VkSurfaceKHR surface);

 private:  // features, extensions and queue families
  QueueFamily m_qfam;
  VkQueue retrieve_queue(QFAM qfam);
  VkQueue retrieve_queue(QFAM qfam, uint32_t qidx);
  std::optional<QueueFamilyEntry> get_queue_family_index(QFAM qfam);
  QueueFamily get_queue_family(VkPhysicalDevice device, VkSurfaceKHR surface);

  std::vector<const char*> m_ext;
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

 private:  // queues
  VkQueue m_gfx_queue = VK_NULL_HANDLE;
  VkQueue m_pre_queue = VK_NULL_HANDLE;
};

}  // namespace xev
