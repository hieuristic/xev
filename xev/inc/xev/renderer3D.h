#pragma once
#include <xev/renderer.h>

namespace xev {

class Renderer3D : Renderer {
public:
  Renderer3D(std::shared_ptr<Backend> backend, std::unique_ptr<Shader> shader,
             const Scene &scene);
  ~Renderer3D();

  void draw(VkCommandBuffer cmd, const Backend &backend, const Scene &scene);
  void drawMesh(VkCommandBuffer cmd, const Backend &backend,
                const Scene &scene);
  void drawSkinnedMesh(VkCommandBuffer cmd, const Backend &backend,
                       const Scene &scene);
}

} // namespace xev
