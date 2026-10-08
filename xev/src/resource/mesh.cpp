#include <xev/hot_exec.h>
#include <xev/logger.h>
#include <xev/resource/mesh.h>
#include <xev/resource_manager.h>
#include <xev/frame_context.h>

namespace xev {

Mesh::Mesh(std::string name,
           glm::mat4 model_mat,
           uint32_t mat_id,
           std::vector<glm::vec3> positions,
           std::vector<glm::vec3> normals,
           std::vector<glm::vec2> uvs,
           std::vector<glm::uvec3> faces,
           std::vector<VertexPalette> skinning)
    : m_name(std::move(name)),
      m_model_mat(model_mat),
      m_mat_id(mat_id),
      m_positions(std::move(positions)),
      m_normals(std::move(normals)),
      m_uvs(std::move(uvs)),
      m_faces(std::move(faces)),
      m_skinning(std::move(skinning)),
{}

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

uint32_t Mesh::get_vertex_count() const {
  return static_cast<uint32_t>(m_positions.size());
}

VkDeviceAddress Mesh::get_vert_addr() const {
  return m_bufVert.addr;
}

VkDeviceAddress Mesh::get_palette_addr() const {
  return m_bufPalette.addr;
}

VkDeviceAddress Mesh::get_skinned_vert_addr(uint32_t frameIdx) const {
  XEV_ASSERT(frameIdx < FrameContext::MAX_IN_FLIGHT);
  return m_bufArrSkinnedVert.get_addr(frameIdx);
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

  // face (index) buffer
  m_bufFace.set_size(sizeof(glm::uvec3) * m_faces.size());
  manager.alloc(m_bufFace);

  // vertex buffer
  m_bufVert.set_size(m_positions.size() * sizeof(Vertex));
  manager.alloc(m_bufVert);

  if (is_skinned()) {
    // skinning buffer
    m_bufPalette.set_size(m_skinning.size() * sizeof(VertexPalette));
    manager.alloc(m_bufPalette);

    // skinned vert buffer
    m_bufArraySkinnedVert.set_size(m_positions.size() * sizeof(Vertex));
    mananger.alloc(m_bufArraySkinnedVert);
  }

  m_on_device = true;
  XEV_INFO("Mesh '{}' reserved on GPU ({} verts, {} faces)", m_name,
           m_positions.size(), m_faces.size());
}

void Mesh::upload(const ResourceManager& manager, const HotExec& hot_exec) {
  XEV_ASSERT(m_bufFace.on_device() && m_bufVert.on_device());

  Arena staging{m_bufFace.size + m_bufVert.size + m_bufPalette.size,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO};
  manager.alloc(staging);

  std::vector<Vertex> vert_data(m_positions.size(), Vertex{});
  for (uint32_t i = 0; i < vert_data.size(); ++i) {
    vert_data[i].position = m_positions[i];
    vert_data[i].normal = m_normals[i];
    vert_data[i].uv = m_uvs[i];
  }

  staging.write(m_faces.data(), m_bufFace.size);
  staging.write(vert_data.data(), m_bufVert.size);
  if (is_skinned()) staging.write(m_skinning.data(), m_bufPalette.size);

  hot_exec.run([&](const VkCommandBuffer cmdbuf) {
    const VkBufferCopy face_reg = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = m_bufFace.size,
    };
    const VkBufferCopy vert_reg = {
        .srcOffset = m_bufFace.size,
        .dstOffset = 0,
        .size = m_bufVert.size,
    };

    vkCmdCopyBuffer(cmdbuf, staging.buffer.buffer, m_bufFace.buffer, 1,
                    &face_reg);
    vkCmdCopyBuffer(cmdbuf, staging.buffer.buffer, m_bufVert.buffer, 1,
                    &vert_reg);
    if (is_skinned()) {
      const VkBufferCopy skin_reg = {
          .srcOffset = m_bufFace.size + m_bufVert.size,
          .dstOffset = 0,
          .size = skin_size,
      };
      vkCmdCopyBuffer(cmdbuf, staging.buffer.buffer, m_bufPalette.buffer, 1,
                      &skin_reg);
      vkCmdCopyBuffer(cmdbuf, staging.buffer.buffer,
                      m_bufArraySkinnedVert.buffer, 1, &vert_reg);
    }
  });

  manager.free(staging);
}

void Mesh::bind(const VkCommandBuffer& cmdbuf, VkDeviceAddress& addr) const {
  vkCmdBindIndexBuffer(cmdbuf, m_bufFace.buffer, 0, VK_INDEX_TYPE_UINT32);
  addr = m_bufVert.addr;
}

bool Mesh::on_device() const {
  return m_on_device;
}

void Mesh::free(const ResourceManager& manager) {
  manager.free(m_bufFace);
  manager.free(m_bufVert);
  if (is_skinned()) {
    manager.free(m_bufPalette);
    manager.free(m_bufArraySkinnedVert);
  }
  m_on_device = false;
}

uint64_t Mesh::size_device() const {
  uint32_t size = m_bufVert.size_device() + m_bufFace.size_device();
  if (is_skinned()) {
    size += m_bufPalette.size_device() + m_bufArraySkinnedVert.size_device();
  }
  return size;
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

void Mesh::write(std::ofstream& out) {
  ;  // TODO IMPLEMENT THIS!
}

}  // namespace xev
