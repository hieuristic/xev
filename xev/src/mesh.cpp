#include <xev/backend.h>
#include <xev/logger.h>
#include <xev/mesh.h>

namespace xev {

Mesh::Mesh(std::string name, glm::vec3 pos, glm::quat rot, glm::vec3 scale,
           std::vector<MeshPrimitive> primitives,
           std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals,
           std::vector<glm::vec2> uvs, std::vector<glm::uvec3> faces)
    : m_name(std::move(name)), m_pos(pos), m_rot(rot), m_scale(scale),
      m_pris(std::move(primitives)), m_vertices(std::move(vertices)),
      m_normals(std::move(normals)), m_uvs(std::move(uvs)),
      m_faces(std::move(faces)) {}

void Mesh::toDevice(const Backend &backend) {
  if (m_is_on_gpu) {
    XEV_WARN("Mesh '{}' already on GPU, skipping toDevice", m_name);
    return;
  }

  // vertex buffer
  if (!m_vertices.empty()) {
    VkDeviceSize size = sizeof(glm::vec3) * m_vertices.size();
    m_gpu_vert = backend.create_buffer(
        m_name + "_vert", size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO);
    if (m_gpu_vert.alloc_info.pMappedData) {
      memcpy(m_gpu_vert.alloc_info.pMappedData, m_vertices.data(), size);
    }
  }

  // normal buffer
  if (!m_normals.empty()) {
    VkDeviceSize size = sizeof(glm::vec3) * m_normals.size();
    m_gpu_norm = backend.create_buffer(
        m_name + "_norm", size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO);
    if (m_gpu_norm.alloc_info.pMappedData) {
      memcpy(m_gpu_norm.alloc_info.pMappedData, m_normals.data(), size);
    }
  }

  // uv buffer
  if (!m_uvs.empty()) {
    VkDeviceSize size = sizeof(glm::vec2) * m_uvs.size();
    m_gpu_uv = backend.create_buffer(
        m_name + "_uv", size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO);
    if (m_gpu_uv.alloc_info.pMappedData) {
      memcpy(m_gpu_uv.alloc_info.pMappedData, m_uvs.data(), size);
    }
  }

  // face (index) buffer
  if (!m_faces.empty()) {
    VkDeviceSize size = sizeof(glm::uvec3) * m_faces.size();
    m_gpu_face = backend.create_buffer(
        m_name + "_face", size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO);
    if (m_gpu_face.alloc_info.pMappedData) {
      memcpy(m_gpu_face.alloc_info.pMappedData, m_faces.data(), size);
    }
  }

  m_is_on_gpu = true;
  XEV_INFO("Mesh '{}' uploaded to GPU ({} verts, {} faces)",
           m_name, m_vertices.size(), m_faces.size());
}

glm::mat4 Mesh::get_model_mat() const {
  glm::mat4 T = glm::translate(glm::mat4(1.0f), m_pos);
  glm::mat4 R = glm::mat4_cast(m_rot);
  glm::mat4 S = glm::scale(glm::mat4(1.0f), m_scale);
  return T * R * S;
}

} // namespace xev
