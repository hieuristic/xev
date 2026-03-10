#pragma once
#include <memory>
#include <xev/pipelines/pipeline.h>
#include <xev/scene.h>
#include <xev/shader.h>
#include <xev/volk.h>

namespace xev {

class Backend;

class Renderer {
public:
  virtual void draw();

private: // backend stuff
  virtual void create_pipeline(const Backend &backend);
  virtual void recreate_pipeline(const Backend &backend);
  Pipeline m_pipeline;
  VkCommandPool m_cmd_pool = VK_NULL_HANDLE;
  VkCommandBuffer m_cmd_buffer = VK_NULL_HANDLE;

private: // synchronization primitives
  VkSemaphore m_sem_image = VK_NULL_HANDLE;
  VkSemaphore m_sem_drawn = VK_NULL_HANDLE;
  VkFence m_fence_inflight;

private:
  static const MAX_LIGHTS = 1000;
  static const MAX_SHADOW_LIGHTS = 3;
};

} // namespace xev
