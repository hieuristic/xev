#pragma once
#include <xev/resource/buffer.h>
#include <xev/resource/resource.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

namespace xev {

class Backend;

struct MeshPrimitive {
  uint32_t voffset;
  uint32_t vlength;
  uint32_t foffset;
  uint32_t flength;
  uint32_t mat_idx;
};

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 uv;
};

class Mesh : Resource {
 public:
  Mesh(std::string name,
       glm::mat4 model_mat,
       std::vector<MeshPrimitive> primitives,
       std::vector<glm::vec3> positions,
       std::vector<glm::vec3> normals,
       std::vector<glm::vec2> uvs,
       std::vector<glm::uvec3> faces);

  const std::string& get_name() const { return m_name; }
  glm::mat4 get_model_mat() const { return m_model_mat; };

  uint64_t size_device() const override;
  uint64_t size_host() const override;

  bool is_loaded() const override { return m_is_loaded; }
  void load(const Backend& backend) override;
  void unload(const Backend& backend) override;

  VkBuffer get_face_buffer() { return m_device_face.buffer; }
  VkBuffer get_vert_buffer() { return m_device_vert.buffer; };

  bool has_skeleton = false;

 private:
  bool m_is_loaded = false;
  std::string m_name;
  glm::mat4 m_model_mat{1.0f};
  std::vector<MeshPrimitive> m_pris;

  // device data
  Buffer m_device_face{0, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                       VMA_MEMORY_USAGE_AUTO};
  Buffer m_device_vert{0, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                       VMA_MEMORY_USAGE_AUTO};

  // host data
  std::vector<glm::uvec3> m_faces;
  std::vector<glm::vec3> m_positions;
  std::vector<glm::vec3> m_normals;
  std::vector<glm::vec2> m_uvs;
};
}  // namespace xev
