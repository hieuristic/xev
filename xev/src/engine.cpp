#include <xev/device.h>
#include <xev/engine.h>
#include <xev/pipeline_manager.h>
#include <xev/resource_manager.h>
#include <xev/swapchain.h>

namespace xev {

Engine::Engine() {
  m_device = std::make_unique<Device>();
}

Engine::Engine(SDL_Window* window) {
  m_device = std::make_unique<Device>(window);
}

void Engine::init_swapchain() {
  swapchain = std::make_unique<Swapchain>(
      m_device.physical_device, m_device.surface, m_device.logical_device,
      m_device.queue_family);
}

void Engine::init_resource_manager() {
  resource_manager = std::make_unique<ResourceManager>(
      m_device.instance, m_device.physical_device, m_device.logical_device);
}

void Engine::init_pipeline_manager();
void Engine::init_global_descriptor_set();

}  // namespace xev
