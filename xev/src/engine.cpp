#include <xev/common.h>
#include <xev/device.h>
#include <xev/engine.h>
#include <xev/filesystem/fs.h>
#include <xev/frame_context.h>
#include <xev/global_descriptor_set.h>
#include <xev/logger.h>
#include <xev/pipeline_manager.h>
#include <xev/resource_manager.h>
#include <xev/swapchain.h>

namespace xev {

Engine::Engine() {
  m_device = std::make_unique<Device>();
  init_resource_manager();
  init_hot_exec();
}

Engine::Engine(SDL_Window* window) {
  m_device = std::make_unique<Device>(window);
  init_resource_manager();
}

void Engine::init_swapchain() {
  if (swapchain != nullptr)
    return;

  XEV_ASSERT(m_device->surface != VK_NULL_HANDLE,
             "Failed to initialize swapchain: invalid surface");

  swapchain = std::make_unique<Swapchain>(
      m_device->physical_device, m_device->surface, m_device->device,
      m_device->queue_family.graphics.value().idx,
      m_device->queue_family.present.value().idx);
}

void Engine::init_hot_exec() {
  if (hotExec != nullptr)
    return;

  hotExec =
      std::make_unique<HotExec>(m_device->device, m_device->graphics_queue,
                                m_device->queue_family.graphics.value().idx);
}

void Engine::init_resource_manager() {
  if (resourceManager != nullptr)
    return;

  resourceManager = std::make_unique<ResourceManager>(
      m_device->instance, m_device->physical_device, m_device->device);
}

void Engine::init_pipeline_manager() {
  if (pipelineManager != nullptr)
    return;

  XEV_ASSERT(globalDescriptorSet != nullptr, "Invalid global desc set!");

  pipelineManager = std::make_unique<PipelineManager>(
      m_device->device, globalDescriptorSet->get_layout());
}

void Engine::init_global_descriptor_set() {
  if ((globalDescriptorSet != nullptr) || (resourceManager == nullptr))
    return;

  globalDescriptorSet = std::make_unique<GlobalDescriptorSet>(m_device->device);
}

void Engine::init_frame_context() {
  if (resourceManager == nullptr)
    init_resource_manager();

  if (swapchain == nullptr)
    init_swapchain();

  frameContext = std::make_unique<FrameContext>(
      m_device->device, m_device->queue_family.graphics.value().idx,
      *resourceManager, swapchain->width, swapchain->height);
}

void Engine::init_file_system() {
  fileSystem = std::make_unique<FileSystem>();
  fileSystem->init_thread_pool();
}

void Engine::submit_and_show(VkCommandBuffer cmd, const Image& image) {
  const auto& frame = frameContext->get_current_frame();

  const Image& swapchain_img = swapchain->acquire_image(frame.present_sem);

  vkResetFences(m_device->device, 1, &frame.render_fence);

  if (image.image != VK_NULL_HANDLE) {
    const_cast<Image&>(swapchain_img)
        .update_layout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    const_cast<Image&>(swapchain_img).blit_from(cmd, image, VK_FILTER_LINEAR);
  }

  const_cast<Image&>(swapchain_img)
      .update_layout(cmd, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  VkResult res_ = vkEndCommandBuffer(cmd);
  XEV_ASSERT_VK(res_, "Failed to end command buffer");

  VkSemaphoreSubmitInfo wait_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
      .semaphore = frame.present_sem,
      .value = 1,
      .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
  };
  VkSemaphoreSubmitInfo signal_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
      .semaphore = frame.render_sem,
      .value = 1,
      .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
  };
  VkCommandBufferSubmitInfo cmd_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
      .commandBuffer = cmd,
  };
  VkSubmitInfo2 submit_info = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
      .waitSemaphoreInfoCount = 1,
      .pWaitSemaphoreInfos = &wait_info,
      .commandBufferInfoCount = 1,
      .pCommandBufferInfos = &cmd_info,
      .signalSemaphoreInfoCount = 1,
      .pSignalSemaphoreInfos = &signal_info,
  };

  VkQueue graphics_queue;
  vkGetDeviceQueue(m_device->device,
                   m_device->queue_family.graphics.value().idx, 0,
                   &graphics_queue);

  res_ = vkQueueSubmit2(graphics_queue, 1, &submit_info, frame.render_fence);
  XEV_ASSERT_VK(res_, "Failed to submit queue");

  swapchain->present(graphics_queue, frame.render_sem);

  frameContext->release_frame();
}

Engine::~Engine() = default;
Engine::Engine(Engine&&) = default;
Engine& Engine::operator=(Engine&&) = default;

}  // namespace xev
