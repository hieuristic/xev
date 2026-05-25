#pragma once
#include <xev/volk.h>
#include <xev/resource/image.h>
#include <vector>

namespace xev {

class Swapchain {
 public:
  Swapchain(VkPhysicalDevice physical_device,
            VkSurfaceKHR surface,
            VkDevice device,
            uint32_t graphics_family_idx,
            uint32_t present_family_idx);
  ~Swapchain();

  Swapchain(const Swapchain&) = delete;
  Swapchain& operator=(const Swapchain&) = delete;
  Swapchain(Swapchain&&) = default;
  Swapchain& operator=(Swapchain&&) = default;

  uint32_t width{0};
  uint32_t height{0};
  bool m_is_swapchain_dirty{false};

  const Image& acquire_image(VkSemaphore swapchain_sem);
  void present(VkQueue queue, VkSemaphore wait_sem);
  void reinit_swapchain();

  VkSwapchainKHR get_swapchain() const { return m_swapchain; }

 private:
  VkDevice m_device{VK_NULL_HANDLE};
  VkPhysicalDevice m_physical_device{VK_NULL_HANDLE};
  VkSurfaceKHR m_surface{VK_NULL_HANDLE};
  uint32_t m_graphics_family_idx{0};
  uint32_t m_present_family_idx{0};
  VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
  VkSurfaceFormatKHR m_ideal_format{
      .format = VK_FORMAT_B8G8R8A8_SRGB,
      .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
  };
  VkSurfaceFormatKHR m_surface_format{};

  uint32_t m_acquired_idx{0};
  std::vector<Image> m_images;

  void init_swapchain();
  void create_images();
  void destroy_images();
};

}  // namespace xev
