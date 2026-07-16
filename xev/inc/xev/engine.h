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
  std::unique_ptr<ResourceManager> resource_manager;
  std::unique_ptr<PipelineManager> pipeline_manager;
  std::unique_ptr<GlobalDescriptorSet> global_descriptor_set;
  std::unique_ptr<FrameContext> frame_context;
  std::unique_ptr<HotExec> hot_exec;

  void init_swapchain();
  void init_hot_exec();
  void init_resource_manager();
  void init_pipeline_manager();
  void init_global_descriptor_set();
  void init_frame_context();
  void leave_frame(VkCommandBuffer cmd, const Image& image);
  void init_renderer();
};

}  // namespace xev
