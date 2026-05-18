#pragma once
#include <xev/geometry/aabb.h>
#include <xev/geometry/sphere.h>
#include <xev/resource/buffer.h>
#include <xev/resource/resource.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

namespace xev {

class Backend;

class Mesh : Resource {
 public:
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

 public:
  Mesh() = default;
  Mesh(std::string name,
       glm::mat4 model_mat,
       std::vector<MeshPrimitive> primitives,
       std::vector<glm::vec3> positions,
       std::vector<glm::vec3> normals,
       std::vector<glm::vec2> uvs,
       std::vector<glm::uvec3> faces);

  const std::string& get_name() const;
  glm::mat4 get_model_mat() const;
  const std::vector<MeshPrimitive>& get_primitives() const;

  uint64_t size_device() const override;

  bool is_reserved() const override;
  void reserve(const Backend& backend) override;
  void release(const Backend& backend) override;

  VkBuffer get_face_buffer() const { return m_device_face.buffer; }
  VkBuffer get_vert_buffer() const { return m_device_vert.buffer; };

  // bounding structure queries
 public:
  bool has_bs() const { return m_has_bs; }
  void get_bs(Sphere& sphere) const;
  void compute_bs();
  bool has_aabb() const { return m_has_aabb; }
  void get_aabb(AABB& aabb) const;
  void compute_aabb();

 private:
  bool m_has_bs = false;
  bool m_has_aabb = false;
  Sphere m_bs;
  AABB m_aabb;

 public:
  bool has_skeleton = false;

 private:
  bool m_is_reserved = false;
  std::string m_name;
  glm::mat4 m_model_mat{1.0f};
  std::vector<MeshPrimitive> m_primitives;

  // device data
  Buffer m_device_face{VK_BUFFER_USAGE_INDEX_BUFFER_BIT};
  Buffer m_device_vert{VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};

  // host data
  std::vector<glm::uvec3> m_faces;
  std::vector<glm::vec3> m_positions;
  std::vector<glm::vec3> m_normals;
  std::vector<glm::vec2> m_uvs;
};
}  // namespace xev
