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

void Mesh::load(const Backend& backend) {
  if (m_is_loaded) {
    XEV_WARN("Mesh '{}' already on GPU, skipping  upload", m_name);
    return;
  }

  if (m_faces.empty() || m_positions.empty()) {
    XEV_WARN("EMPTY mesh '{}' is getting binded, doing nothing", m_name);
    return;
  }

  uint64_t size;

  // face (index) buffer
  size = sizeof(glm::uvec3) * m_faces.size();
  m_device_face.load(size, backend);
  m_device_face.copy(m_faces.data(), 0, size, backend);

  // vertex buffer
  size = m_positions.size() * sizeof(Vertex);
  std::vector<Vertex> interleaved_vertex_data(m_positions.size(), Vertex{});
  for (uint32_t i = 0; i < interleaved_vertex_data.size(); ++i) {
    interleaved_vertex_data[i].position = m_positions[i];
    interleaved_vertex_data[i].normal = m_normals[i];
    interleaved_vertex_data[i].uv = m_uvs[i];
  }
  m_device_vert.load(size, backend);
  m_device_vert.copy(interleaved_vertex_data.data(), 0, size, backend);

  m_is_loaded = true;
  XEV_INFO("Mesh '{}' uploaded to GPU ({} verts, {} faces)", m_name,
           m_positions.size(), m_faces.size());
}

bool Mesh::is_loaded() const {
  return m_is_loaded;
}

void Mesh::unload(const Backend& backend) {
  m_device_face.unload(backend);
  m_device_vert.unload(backend);
  m_is_loaded = false;
}

uint64_t Mesh::size_device() const {
  return m_device_vert.size_device() + m_device_face.size_device();
};

uint64_t Mesh::size_host() const {
  return m_faces.size() * sizeof(m_faces[0]) +
         m_positions.size() * sizeof(m_positions[0]) +
         m_normals.size() * sizeof(m_normals[0]) +
         m_uvs.size() * sizeof(m_uvs[0]);
}

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
