#pragma once

#include <queue>
#include <string>
#include <string_view>
#include <vector>
#include <glm/glm.hpp>
#include <xev/camera.h>
#include <xev/mesh.h>

namespace tinygltf {
struct Model;
struct Node;
} // namespace tinygltf

namespace xev {

class Scene {
public:
  Scene();
  void load_gltf(std::string_view filepath);
  void create_test_triangle();

  std::vector<glm::vec3> m_vert_buffer;
  std::vector<glm::vec3> m_norm_buffer;
  std::vector<glm::vec2> m_uv_buffer;
  std::vector<glm::uvec3> m_face_buffer;
  std::vector<Mesh> m_meshes;
  Camera m_active_cam;

private:
  Mesh parse_mesh(const tinygltf::Model &model, const tinygltf::Node &node);
};

} // namespace xev
