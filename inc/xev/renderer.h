#pragma once
#include <memory>
#include <volk.h>
#include <xev/shader.h>
#include <xev/scene.h>
#include <xev/backend.h>

namespace xev {

class Renderer {
public:
  Renderer(std::shared_ptr<Backend> backend, std::unique_ptr<Shader> shader, const Scene& scene);
  ~Renderer();

  void draw();
  void draw_triangle();

private:
  void create_pipeline(VkDevice device, VkFormat swapchain_format);
  void recreate_pipeline(VkDevice device);

  uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

  std::shared_ptr<Backend> m_backend;
  std::unique_ptr<Shader> m_shader;
  const Scene* m_scene = nullptr;
  VkPipeline m_pipeline = VK_NULL_HANDLE;
  VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
  VkDescriptorSetLayout m_cam_ds_layout = VK_NULL_HANDLE;

  VkCommandPool m_cmd_pool = VK_NULL_HANDLE;
  VkCommandBuffer m_cmd_buffer = VK_NULL_HANDLE;

  VkSemaphore m_image_available_sem = VK_NULL_HANDLE;
  VkSemaphore m_render_finished_sem = VK_NULL_HANDLE;
  VkFence m_in_flight_fence = VK_NULL_HANDLE;

  VkBuffer m_vertex_buffer = VK_NULL_HANDLE;
  VkDeviceMemory m_vertex_buffer_memory = VK_NULL_HANDLE;

  VkBuffer m_index_buffer = VK_NULL_HANDLE;
  VkDeviceMemory m_index_buffer_memory = VK_NULL_HANDLE;
  uint32_t m_index_count = 0;
};

} // namespace xev
