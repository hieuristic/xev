#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>
#include <xev/buffer.h>

namespace xev {

class Backend;

struct MeshPrimitive {
  uint32_t voffset;
  uint32_t vlength;
  uint32_t foffset;
  uint32_t flength;
  uint32_t mat_idx;
};

class Mesh {
public:
  Mesh(std::string name, glm::vec3 pos, glm::quat rot, glm::vec3 scale,
       std::vector<MeshPrimitive> primitives,
       std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals,
       std::vector<glm::vec2> uvs, std::vector<glm::uvec3> faces);

  const std::string &get_name() const { return m_name; }
  glm::mat4 get_model_mat() const;
  const std::vector<MeshPrimitive> &get_primitives() const { return m_pris; }
  void toDevice(const Backend &backend);

  bool has_skeleton = false;

private:
  bool m_is_on_gpu = false;
  std::string m_name;
  glm::vec3 m_pos{0.0f};
  glm::quat m_rot{1.0f, 0.0f, 0.0f, 0.0f};
  glm::vec3 m_scale{1.0f};
  std::vector<MeshPrimitive> m_pris;

  // host data
  std::vector<glm::vec3> m_vertices;
  std::vector<glm::vec3> m_normals;
  std::vector<glm::vec2> m_uvs;
  std::vector<glm::uvec3> m_faces;

  // gpu buffers
  Buffer m_gpu_vert{};
  Buffer m_gpu_norm{};
  Buffer m_gpu_uv{};
  Buffer m_gpu_face{};
};
} // namespace xev
