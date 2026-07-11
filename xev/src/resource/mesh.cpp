#include <xev/logger.h>
#include <xev/hot_exec.h>
#include <xev/resource/mesh.h>
#include <xev/resource_manager.h>

namespace xev {

Mesh::Mesh(std::string name,
           glm::mat4 model_mat,
           uint32_t mat_id,
           std::vector<glm::vec3> positions,
           std::vector<glm::vec3> normals,
           std::vector<glm::vec2> uvs,
           std::vector<glm::uvec3> faces)
    : m_name(std::move(name)),
      m_model_mat(model_mat),
      m_mat_id(mat_id),
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

uint32_t Mesh::get_material_id() const {
  return m_mat_id;
}

uint32_t Mesh::get_face_count() const {
  return static_cast<uint32_t>(m_faces.size());
}

VkDeviceAddress Mesh::get_vert_addr() const {
  return m_device_vert.addr;
}

void Mesh::alloc(const ResourceManager& manager) {
  if (m_on_device) {
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

void Mesh::upload(const ResourceManager& manager, const HotExec& hot_exec) {
  XEV_ASSERT(m_device_face.on_device() && m_device_vert.on_device());

  Buffer staging{m_device_face.size + m_device_vert.size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO};
  manager.alloc(staging);

  std::vector<Vertex> vert_data(m_positions.size(), Vertex{});
  for (uint32_t i = 0; i < vert_data.size(); ++i) {
    vert_data[i].position = m_positions[i];
    vert_data[i].normal = m_normals[i];
    vert_data[i].uv = m_uvs[i];
  }

  void* map_ = staging.alloc_info.pMappedData;
  memcpy(map_, m_faces.data(), m_device_face.size);
  memcpy((char*)map_ + m_device_face.size, vert_data.data(),
         m_device_vert.size);

  hot_exec.run([&](const VkCommandBuffer cmdbuf) {
    const VkBufferCopy face_reg = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = m_device_face.size,
    };
    const VkBufferCopy vert_reg = {
        .srcOffset = m_device_face.size,
        .dstOffset = 0,
        .size = m_device_vert.size,
    };

    vkCmdCopyBuffer(cmdbuf, staging.buffer, m_device_face.buffer, 1, &face_reg);
    vkCmdCopyBuffer(cmdbuf, staging.buffer, m_device_vert.buffer, 1, &vert_reg);
  });

  manager.free(staging);
}

void Mesh::bind(const VkCommandBuffer& cmdbuf, VkDeviceAddress& addr) const {
  vkCmdBindIndexBuffer(cmdbuf, m_device_face.buffer, 0, VK_INDEX_TYPE_UINT32);
  addr = m_device_vert.addr;
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
