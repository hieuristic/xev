#include <xev/backend.h>
#include <xev/logger.h>
#include <xev/resource/mesh.h>

namespace xev {

Mesh::Mesh(std::string name,
           glm::mat4 model_mat,
           std::vector<MeshPrimitive> primitives,
           std::vector<glm::vec3> positions,
           std::vector<glm::vec3> normals,
           std::vector<glm::vec2> uvs,
           std::vector<glm::uvec3> faces)
    : m_name(std::move(name)),
      m_model_mat(model_mat),
      m_primitives(std::move(primitives)),
      m_positions(std::move(positions)),
      m_normals(std::move(normals)),
      m_uvs(std::move(uvs)),
      m_faces(std::move(faces)) {}

const std::string& Mesh::get_name() const {
  return m_name;
}
glm::mat4 Mesh::get_model_mat() const {
  return m_model_mat;
};
const std::vector<Mesh::MeshPrimitive>& Mesh::get_primitives() const {
  return m_primitives;
}

void Mesh::reserve(const Backend& backend) {
  if (m_is_reserved) {
    XEV_WARN("Mesh '{}' already on GPU, skipping reserve", m_name);
    return;
  }

  if (m_faces.empty() || m_positions.empty()) {
    XEV_WARN("EMPTY mesh '{}' is getting binded, doing nothing", m_name);
    return;
  }

  uint64_t size;

  // face (index) buffer
  size = sizeof(glm::uvec3) * m_faces.size();
  m_device_face.reserve(size, backend);

  // vertex buffer
  size = m_positions.size() * sizeof(Vertex);
  m_device_vert.reserve(size, backend);
  m_device_vert.copy(interleaved_vertex_data.data(), 0, size, backend);

  m_is_reserved = true;
  XEV_INFO("Mesh '{}' reserved on GPU ({} verts, {} faces)", m_name,
           m_positions.size(), m_faces.size());
}

void Mesh::upload(const Backend& backend) {
  uint64_t size;

  XEV_ASSERT(m_device_face.is_reserved());
  size = sizeof(glm::uvec3) * m_faces.size();
  m_device_face.upload(m_faces.data(), 0, size, backend);

  XEV_ASSERT(m_device_vert.is_reserved());
  size = m_positions.size() * sizeof(Vertex);
  std::vector<Vertex> interleaved_vertex_data(m_positions.size(), Vertex{});
  for (uint32_t i = 0; i < interleaved_vertex_data.size(); ++i) {
    interleaved_vertex_data[i].position = m_positions[i];
    interleaved_vertex_data[i].normal = m_normals[i];
    interleaved_vertex_data[i].uv = m_uvs[i];
  }
  m_device_vert.upload(interleaved_vertex_data.data(), 0, size, backend);
}

bool Mesh::is_reserved() const {
  return m_is_reserved;
}

void Mesh::release(const Backend& backend) {
  m_device_face.release(backend);
  m_device_vert.release(backend);
  m_is_reserved = false;
}

uint64_t Mesh::size_device() const {
  return m_device_vert.size_device() + m_device_face.size_device();
};

void Mesh::get_bs(Sphere& bs) const {
  bs = m_bs;
}

void Mesh::compute_bs() {
  XEV_ERROR("NOT IMPLEMENTED");

  m_has_bs = true;
}

void Mesh::get_aabb(AABB& aabb) const {
  aabb = m_aabb;
}

void Mesh::compute_aabb() {
  XEV_ERROR("NOT IMPLEMENTED");
  m_has_aabb = true;
}

}  // namespace xev
