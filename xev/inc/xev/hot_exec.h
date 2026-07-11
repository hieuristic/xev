#pragma once
#include <xev/volk.h>
#include <functional>

namespace xev {

class HotExec {
 public:
  HotExec(VkDevice device, VkQueue graphics_queue, uint32_t graphics_idx);
  ~HotExec();

  HotExec(const HotExec&) = delete;
  HotExec& operator=(const HotExec&) = delete;
  HotExec(HotExec&&) = default;
  HotExec& operator=(HotExec&&) = default;

  void run(std::function<void(const VkCommandBuffer)> callback) const;

 private:
  VkDevice m_device;
  VkQueue m_queue;
  VkFence m_fence;
  uint32_t m_family_idx;

  VkCommandPool m_pool;
  VkCommandBuffer m_cmdbuf;
};

}  // namespace xev
