#include <xev/renderer3D.h>
#include <xev/scene.h>

namespace xev {

Renderer3D::Renderer3D(std::shared_ptr<Backend> backend, ImageSize size)
    : m_backend(std::move(backend)) {
  create_pipeline_mesh();
}

Renderer3D::~Renderer3D() {}

void draw(VkCommandBuffer cmd, const Scene& scene) {
  // TODO
}

void draw_mesh(VkCommandBuffer cmd, const Scene& scene) {
  // TODO
}

void draw_mesh_skinned(VkCommandBuffer cmd, const Scene& scene) {
  // TODO
}

void create_pipeline_mesh() {
  // TODO
}

void create_pipeline_mesh_skinned() {
  // TODO
}

}  // namespace xev
