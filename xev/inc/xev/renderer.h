#pragma once
#include <memory>
#include <xev/backend.h>
#include <xev/pipelines/pipeline.h>
#include <xev/scene.h>
#include <xev/shader.h>
#include <xev/volk.h>

namespace xev {

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
  static const MAX_LIGHTS = 100;
};

class Renderer2D : Renderer {
public:
  Renderer2D(const Backend &backend);
  void draw();
};

class Renderer3D : Renderer {
public:
  Renderer3D(std::shared_ptr<Backend> backend, std::unique_ptr<Shader> shader,
             const Scene &scene);
  ~Renderer3D();

  void draw();
  void drawMesh();
  void drawFog();
} // namespace xev
