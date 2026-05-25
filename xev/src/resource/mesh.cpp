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

void Mesh::alloc(const ResourceManager& manager) {
  if (m_is_alloced) {
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
  m_device_face.size = size;
  manager.alloc(m_device_face);

  // vertex buffer
  size = m_positions.size() * sizeof(Vertex);
  m_device_vert.size = size;
  manager.alloc(m_device_vert);

  m_on_device = true;
  XEV_INFO("Mesh '{}' reserved on GPU ({} verts, {} faces)", m_name,
           m_positions.size(), m_faces.size());
}

void Mesh::upload(const ResourceManager& manager) {
  XEV_ASSERT(m_device_face.on_device() && m_device_vert.on_device());

  uint64_t size;

  size = sizeof(glm::uvec3) * m_faces.size();
  manager.upload(m_device_face, m_faces.data(), 0, size);

  size = m_positions.size() * sizeof(Vertex);
  std::vector<Vertex> interleaved_vertex_data(m_positions.size(), Vertex{});
  for (uint32_t i = 0; i < interleaved_vertex_data.size(); ++i) {
    interleaved_vertex_data[i].position = m_positions[i];
    interleaved_vertex_data[i].normal = m_normals[i];
    interleaved_vertex_data[i].uv = m_uvs[i];
  }
  manager.upload(m_device_vert, interleaved_vertex_data.data(), 0, size);
}

bool Mesh::on_device() const {
  return m_on_device;
}

void Mesh::free(const ResourceManager& manager) {
  manager.free(m_device_face);
  manager.free(m_device_vert);
  m_on_device = false;
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
