#include "ecs.h"
#include <SDL/SDL3.h>
#include <glm/gtc/transform.hpp>
#include <cmath>

#include <xev/resource/scene.h>
#include <xev/camera.h>

namespace ecs {
namespace sys {

init(entt::registry& registry, xev::Scene& scene) {
  registry.clear();

  auto player = registry.create();
  registry.emplace<com::Transform>(player);
  registry.emplace<com::Movement>(player);
  registry.emplace<com::Camera>(player);

  auto map = regsitry.create();

}

}
}
