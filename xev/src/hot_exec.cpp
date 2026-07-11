#include <xev/hot_exec.h>
#include <xev/logger.h>
#include <cstdint>

namespace xev {

HotExec::HotExec(VkDevice device, VkQueue graphics_queue, uint32_t graphics_idx)
    : m_device(device), m_queue(graphics_queue), m_family_idx(graphics_idx) {
  VkResult res_;

  {  // command pool
    VkCommandPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_family_idx,
    };
    res_ = vkCreateCommandPool(m_device, &info, nullptr, &m_pool);
    XEV_ASSERT_VK(res_, "Failed to create frame command pool");
  }

  {  // command buffer
    VkCommandBufferAllocateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    res_ = vkAllocateCommandBuffers(m_device, &info, &m_cmdbuf);
    XEV_ASSERT_VK(res_, "Failed to allocate frame command buffer");
  }

  {  // fence for command buffer
    VkFenceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    res_ = vkCreateFence(m_device, &info, nullptr, &m_fence);
    XEV_ASSERT_VK(res_, "Failed to create frame render fence");
  }
}

HotExec::~HotExec() {
  vkDestroyFence(m_device, m_fence, nullptr);
  vkDestroyCommandPool(m_device, m_pool, nullptr);
}

void HotExec::run(std::function<void(const VkCommandBuffer)> callback) const {
  VkResult res_;

  VkCommandBufferBeginInfo binfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };

  VkCommandBufferSubmitInfo cmdbuf_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
      .commandBuffer = m_cmdbuf,
  };

  VkSubmitInfo2 sinfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
      .commandBufferInfoCount = 1,
      .pCommandBufferInfos = &cmdbuf_info,
  };

  XEV_ASSERT_VK(vkResetFences(m_device, 1, &m_fence), "Failed to reset fence");
  XEV_ASSERT_VK(vkResetCommandBuffer(m_cmdbuf, 0), "Failed to reset cmdbuf");
  XEV_ASSERT_VK(vkBeginCommandBuffer(m_cmdbuf, &binfo), "Failed to begin");

  callback(m_cmdbuf);

  XEV_ASSERT_VK(vkEndCommandBuffer(m_cmdbuf), "Failed to end ");
  XEV_ASSERT_VK(vkQueueSubmit2(m_queue, 1, &sinfo, m_fence),
                "Failed to submit");
  XEV_ASSERT_VK(vkWaitForFences(m_device, 1, &m_fence, true, UINT64_MAX),
                "Failed to reset fence");
}

}  // namespace xev
