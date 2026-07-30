#include <xev/frame_context.h>
#include <xev/resource_manager.h>
#include <xev/logger.h>

namespace xev {

FrameContext::FrameContext(VkDevice device,
                           uint32_t graphics_family_idx,
                           const ResourceManager& manager,
                           uint32_t width_,
                           uint32_t height_)
    : m_device(device),
      m_family_idx(graphics_family_idx),
      m_manager(manager),
      width(width_),
      height(height_),
      m_curr_idx(0) {
  for (auto& frame : m_frames) {
    create_frame(frame);
  }
}

FrameContext::~FrameContext() {
  for (auto& frame : m_frames) {
    destroy_frame(frame);
  }
}

void FrameContext::create_frame(Frame& frame) {
  VkResult res_;

  {  // command pool
    VkCommandPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_family_idx,
    };
    res_ = vkCreateCommandPool(m_device, &info, nullptr, &frame.pool);
    XEV_ASSERT_VK(res_, "Failed to create frame command pool");
  }

  {  // command buffer
    VkCommandBufferAllocateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = frame.pool,
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

  frame.render_target.width = width;
  frame.render_target.height = height;
  m_manager.alloc(frame.render_target);

  frame.render_depth.width = width;
  frame.render_depth.height = height;
  m_manager.alloc(frame.render_depth);
}

void FrameContext::destroy_frame(Frame& frame) {
  vkDestroyFence(m_device, frame.render_fence, nullptr);
  vkDestroySemaphore(m_device, frame.render_sem, nullptr);
  vkDestroySemaphore(m_device, frame.present_sem, nullptr);
  vkDestroyCommandPool(m_device, frame.pool, nullptr);
  m_manager.free(frame.render_depth);
  m_manager.free(frame.render_target);
}

VkCommandBuffer FrameContext::acquire_frame() {
  VkResult res_;
  const Frame& frame = m_frames[m_curr_idx];
  res_ = vkWaitForFences(m_device, 1, &frame.render_fence, VK_TRUE, UINT64_MAX);
  XEV_ASSERT_VK(res_, "Failed to acquire frame");
  vkResetFences(m_device, 1, &frame.render_fence);

  VkCommandBufferBeginInfo info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  res_ = vkBeginCommandBuffer(frame.render_cmdbuf, &info);
  XEV_ASSERT_VK(res_, "Failed to begin command buffer");

  return frame.render_cmdbuf;
}

void FrameContext::release_frame() {
  m_curr_idx = (m_curr_idx + 1) % MAX_IN_FLIGHT;
}

}  // namespace xev
