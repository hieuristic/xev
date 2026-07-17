#pragma once
#include <xev/resource/image.h>
#include <xev/volk.h>
#include <array>
#include <cstdint>

namespace xev {

class ResourceManager;

class FrameContext {
 public:
  struct Frame {
    VkCommandPool pool{VK_NULL_HANDLE};
    VkCommandBuffer render_cmdbuf{VK_NULL_HANDLE};
    VkSemaphore present_sem{VK_NULL_HANDLE};
    VkSemaphore render_sem{VK_NULL_HANDLE};
    VkFence render_fence{VK_NULL_HANDLE};
    Image render_depth{
        VK_FORMAT_D32_SFLOAT,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    };
    Image render_target{
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    };
  };

  FrameContext(VkDevice device,
               uint32_t graphics_family_idx,
               const ResourceManager& manager,
               uint32_t width_,
               uint32_t height_);
  ~FrameContext();

  inline static constexpr uint32_t max_in_flight = 2;
  std::array<float, 4> clear_color{0.5f, 0.5f, 0.5f, 1.0f};

  uint32_t width{0};
  uint32_t height{0};

  VkCommandBuffer acquire_frame();
  void release_frame();

  const Frame& get_current_frame() const { return m_frames[m_curr_idx]; }
  const Image& get_current_render_target() const {
    return m_frames[m_curr_idx].render_target;
  }
  const Image& get_current_render_depth() const {
    return m_frames[m_curr_idx].render_depth;
  }

 private:
  uint32_t m_family_idx;
  VkDevice m_device;
  const ResourceManager& m_manager;

  std::array<Frame, max_in_flight> m_frames;
  uint32_t m_curr_idx;

  void create_frame(Frame& frame);
  void destroy_frame(Frame& frame);
};

}  // namespace xev
