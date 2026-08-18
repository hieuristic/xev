#pragma once
#include <xev/volk.h>
#include <memory>

struct SDL_Window;

namespace xev {

struct Image;
struct Device;
struct Swapchain;
struct ResourceManager;
struct PipelineManager;
struct GlobalDescriptorSet;
struct FrameContext;
struct FileSystem;
struct HotExec;

struct Engine {
  Engine();
  Engine(SDL_Window* window);
  ~Engine();
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine(Engine&&);
  Engine& operator=(Engine&&);

  std::unique_ptr<Swapchain> swapchain;
  std::unique_ptr<ResourceManager> resourceManager;
  std::unique_ptr<PipelineManager> pipelineManager;
  std::unique_ptr<GlobalDescriptorSet> globalDescriptorSet;
  std::unique_ptr<FrameContext> frameContext;
  std::unique_ptr<HotExec> hotExec;
  std::unique_ptr<FileSystem> fileSys;

  void init_swapchain();
  void init_resource_manager();
  void init_pipeline_manager();
  void init_global_descriptor_set();
  void init_frame_context();
  void init_hot_exec();
  void init_file_system();
  void submit_and_show(VkCommandBuffer cmd, const Image& image);

 private:
  std::unique_ptr<Device> m_device;
};

}  // namespace xev
