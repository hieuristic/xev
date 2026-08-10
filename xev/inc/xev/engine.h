#pragma once
#include <xev/volk.h>

struct SDL_Window;

namespace xev {

class Image;
class Device;
class Swapchain;
class ResourceManager;
class PipelineManager;
class GlobalDescriptorSet;
class FrameContext;
class FileSystem;

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
  std::unique_ptr<FileSystem> fileSystem;

  void init_swapchain();
  void init_resource_manager();
  void init_pipeline_manager();
  void init_global_descriptor_set();
  void init_frame_context();
  void init_hot_exec();
  void init_file_system();
  void submit_and_show(VkCommandBuffer cmd, const Image& image);
};

}  // namespace xev
