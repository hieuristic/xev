#include <xev/device.h>
#include <xev/engine.h>
#include <xev/frame_context.h>
#include <xev/pipeline_manager.h>
#include <xev/resource_manager.h>
#include <xev/swapchain.h>

namespace xev {

Engine::Engine() {
  m_device = std::make_unique<Device>();
  init_resource_manager();
}

Engine::Engine(SDL_Window* window) {
  m_device = std::make_unique<Device>(window);
  init_resource_manager();
}

void Engine::init_swapchain() {
  if (swapchain != nullptr) return;
  XEV_ASSERT(m_device.surface != VK_NULL_HANDLE,
             "Failed to initialize swapchain: invalid surface");

  swapchain = std::make_unique<Swapchain>(
      m_device.physical_device, m_device.surface, m_device.logical_device,
      m_device.queue_family.graphics.value().idx,
      m_device.queue_family.present.value().idx);
}

void Engine::init_resource_manager() {
  if (resource_manager != nullptr) return;
  resource_manager = std::make_unique<ResourceManager>(
      m_device.instance, m_device.physical_device, m_device.logical_device);
}

void Engine::init_pipeline_manager() {

}

void Engine::init_global_descriptor_set() {
  ;
}

void Engine::init_frame_context() {
  if (resource_manager == nullptr)
    init_resource_manager();

  if (swapchain == nullptr)
    init_swapchain();

  frame_context = std::make_unique<FrameContext>(
      m_device.device, m_device.queue_family.graphics.value().idx,
      resource_manager, swapchain.width, swapchain.height);
}

}  // namespace xev
