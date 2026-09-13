#include "ecs.h"
#include <SDL3/SDL.h>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

#include <xev/camera.h>
#include <xev/resource/scene.h>

namespace ecs {

void sys::init(entt::registry& registry, xev::Scene& scene) {
  registry.clear();

  auto player = registry.create();
  registry.emplace<com::Player>(player);
  registry.emplace<com::Transform>(player);
  registry.emplace<com::Movement>(player);

  auto map = registry.create();
  registry.emplace<com::Transform>(map);

  // bind meshes to player
  for (uint32_t i = 0; i < scene.meshes.size(); ++i) {
    const auto& mesh = scene.meshes[i];
    auto e = registry.create();
    bool isPlayer = mesh.get_name().rfind("hieu.", 0) == 0;
    registry.emplace<com::Mesh>(e, isPlayer ? player : map, i,
                                mesh.get_model_mat());
  }
}

constexpr glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);
constexpr glm::vec3 right = glm::vec3(1.0f, 0.0, 0.0);
constexpr glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f);

void sys::movement(entt::registry& registry, float dt, const bool* keys) {
  auto view = registry.view<com::Transform, com::Movement, com::Player>();
  for (auto [entity, transform, movement] : view.each()) {
    glm::vec3 moveDir{0.0f};
    if (keys[SDL_SCANCODE_W]) moveDir += forward;
    if (keys[SDL_SCANCODE_S]) moveDir -= forward;
    if (keys[SDL_SCANCODE_A]) moveDir -= right;
    if (keys[SDL_SCANCODE_D]) moveDir += right;
    if (keys[SDL_SCANCODE_SPACE]) moveDir += up;
    if (keys[SDL_SCANCODE_LSHIFT]) moveDir -= up;
    movement.velocity = (glm::length(moveDir) > 0.001f)
                            ? glm::normalize(moveDir) * movement.speed
                            : glm::vec3(0.0f);
    transform.pos += movement.velocity * dt;
  }
}

void sys::transform(entt::registry& registry) {
  auto view = registry.view<com::Transform>();
  for (auto [entity, t] : view.each()) {
    t.world_mat = glm::translate(glm::mat4(1.0f), t.pos) * glm::mat4_cast(t.rot);
  }
}

void sys::render_sync(entt::registry& registry, xev::Scene& scene) {
  auto view = registry.view<com::Mesh>();
  for (auto [entity, mesh_comp] : view.each()) {
    if (!registry.valid(mesh_comp.owner)) continue;
    if (mesh_comp.meshIdx >= scene.meshes.size()) continue;
    const auto& parent = registry.get<com::Transform>(mesh_comp.owner);
    scene.meshes[mesh_comp.meshIdx].set_model_mat(parent.world_mat *
                                                  mesh_comp.localOffset);
  }
}

}  // namespace ecs
