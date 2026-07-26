#pragma once
#include <xev/resource/image.h>
#include <xev/volk.h>
#include <memory>

struct SDL_Window;

namespace xev {

class Device;
class Swapchain;
class ResourceManager;
class PipelineManager;
class GlobalDescriptorSet;
class FrameContext;

class Engine {
 public:
  Engine();
  Engine(SDL_Window* window);
  ~Engine();
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine(Engine&&);
  Engine& operator=(Engine&&);

 private:
  std::unique_ptr<Device> m_device;

 public:
  std::unique_ptr<Swapchain> swapchain;
  std::unique_ptr<ResourceManager> resourceManager;
  std::unique_ptr<PipelineManager> pipelineManager;
  std::unique_ptr<GlobalDescriptorSet> globalDescriptorSet;
  std::unique_ptr<FrameContext> frameContext;
  std::unique_ptr<HotExec> hotExec;

  void init_swapchain();
  void init_hot_exec();
  void init_resource_manager();
  void init_pipeline_manager();
  void init_global_descriptor_set();
  void init_frame_context();
  void submit_and_show(VkCommandBuffer cmd, const Image& image);
  void init_renderer();
};

}  // namespace xev
