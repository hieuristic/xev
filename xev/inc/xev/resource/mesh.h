#pragma once
#include <xev/geometry/aabb.h>
#include <xev/geometry/sphere.h>
#include <xev/resource/resource.h>
#include <xev/resource/buffer.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

namespace xev {

struct ResourceManager;
struct HotExec;

struct Mesh : public Resource {
  struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
  };

  Mesh() = default;
  Mesh(std::string name,
       glm::mat4 model_mat,
       uint32_t mat_id,
       std::vector<glm::vec3> positions,
       std::vector<glm::vec3> normals,
       std::vector<glm::vec2> uvs,
       std::vector<glm::uvec3> faces);

  const std::string& get_name() const;
  glm::mat4 get_model_mat() const;
  VkDeviceAddress get_vert_addr() const;
  uint32_t get_material_id() const;
  uint32_t get_face_count() const;

  uint64_t size_device() const override;
  bool on_device() const override;
  void alloc(const ResourceManager& manager);
  void free(const ResourceManager& manager);
  void upload(const ResourceManager& manager, const HotExec& hot_exec);

  void bind(const VkCommandBuffer& cmdbuf, VkDeviceAddress& address) const;

  // bounding structure queries
  bool has_bs() const { return m_has_bs; }
  void get_bs(Sphere& sphere) const;
  void compute_bs();
  bool has_aabb() const { return m_has_aabb; }
  void get_aabb(AABB& aabb) const;
  void compute_aabb();

  bool has_skeleton = false;

 private:
  bool m_has_bs = false;
  bool m_has_aabb = false;
  Sphere m_bs;
  AABB m_aabb;
  bool m_on_device = false;
  std::string m_name;
  glm::mat4 m_model_mat{1.0f};

  // device data
  Buffer m_device_face{
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE};
  Buffer m_device_vert{VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                           VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                           VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                       VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE};

  // host data
  std::vector<glm::uvec3> m_faces;
  std::vector<glm::vec3> m_positions;
  std::vector<glm::vec3> m_normals;
  std::vector<glm::vec2> m_uvs;
  uint32_t m_mat_id;
};
}  // namespace xev
