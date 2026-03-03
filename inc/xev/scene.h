#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <glm/glm.hpp>
#include <xev/camera.h>

namespace xev {

class Scene {
public:
  Scene();
  void load_gltf(std::string_view filepath);
  void create_test_triangle();

  std::vector<glm::vec3> m_vert_buffer;
  std::vector<glm::uvec3> m_face_buffer;
  Camera m_active_cam;
};

} // namespace xev
