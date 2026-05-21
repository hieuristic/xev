#pragma once
#include <xev/resource/image.h>

namespace xev {

class Swapchain() {
 public:
  Swapchain();
  ~Swapchain();

  Swapchain(const Swapchain&) = delete;
  Swapchain& operator=(const Swapchain&) = delete;
  Swapchain(Swapchain&&) = default;
  Swapchain& operator=(Swapchain&&) = default;

  inline constexpr uint32_t MAX_FRAME_OVERLAP = 2;
  std::array<float, 4> clear_color{0.5f, 0.5f, 0.5f, 1.0f};

  VkCommandBuffer acquire_frame();
  void release_frame();

 private:
  VkDevice m_device{VK_NULL_HANDLE};
  VkPhysicalDevice m_physical_device{VK_NULL_HANDLE};
  VkSurfaceKHR m_surface{VK_NULL_HANDLE};
  QueueFamily m_queue_family{VK_NULL_HANDLE};
  VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
  VkExtent2D m_extent{0, 0};
  VkSurfaceFormatKHR m_ideal_format{
      .format = VK_FORMAT_B8G8R8A8_SRGB,
      .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
  };
  VkSurfaceFormatKHR m_surface_format{};

  uint32_t m_frame_idx{0};
  std::array<Frame, MAX_FRAME_OVERLAP> m_frames;
  std::vector<Image> m_images;

  void init_swapchain();
  void reinit_swapchain();
  void create_images(Frame& frame);
  void destroy_images(Frame& frame);
  void create_frame(Frame& frame);
  void destroy_frame(Frame& frame);
  const Image& acquire_image();

  struct Frame {
    VkCommandPool pool{VK_NULL_HANDLE};
    VkCommandBuffer render_cmdbuf{VK_NULL_HANDLE};
    VkSemaphore present_sem{VK_NULL_HANDLE};
    VkSemaphore render_sem{VK_NULL_HANDLE};
    VkFence render_fence{VK_NULL_HANDLE};
  };
}

}  // namespace xev
