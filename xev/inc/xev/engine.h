#pragma once
#include <unique_ptr>

namespace xev {

class Device;
class Swapchain;
class ResourceManager;
class PipelineManager;
class GlobalDescriptorSet;

class Engine {
 public:
  Engine();
  Engine(SDL_Window* window);
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine(Engine&&) = default;
  Engine& operator=(Engine&&) = default;

 private:
  std::unique_ptr<Device> m_device;

 public:
  std::unique_ptr<Swapchain> swapchain;
  std::unique_ptr<ResourceManager> resource_manager;
  std::unique_ptr<PipelineManager> pipeline_manager;
  std::unique_ptr<GlobalDescriptorSet> global_decriptor_set;

  void init_swapchain();
  void init_resource_manager();
  void init_pipeline_manager();
  void init_global_descriptor_set();
}

}  // namespace xev
