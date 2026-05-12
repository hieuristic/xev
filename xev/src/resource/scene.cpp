#include <xev/logger.h>
#include <xev/resource/scene.h>
#include <glm/gtc/type_ptr.hpp>
#include <limits>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

namespace xev {

Scene::Scene() {}

void Scene::parse_mesh(Mesh& mesh,
                       const tinygltf::Model& model,
                       const tinygltf::Node& node,
                       glm::mat4 model_mat) const {
  std::string name = node.name;

  const tinygltf::Mesh& gltf_mesh = model.meshes[node.mesh];

  std::vector<Mesh::MeshPrimitive> primitives;
  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> uvs;
  std::vector<glm::uvec3> faces;

  for (const tinygltf::Primitive& pri : gltf_mesh.primitives) {
    if (pri.mode != -1 && pri.mode != TINYGLTF_MODE_TRIANGLES) {
      XEV_WARN("Skipping non-triangle primitive in mesh '{}'", name);
      continue;
    }

    auto pos_it = pri.attributes.find("POSITION");
    if (pos_it == pri.attributes.end()) {
      XEV_WARN("Primitive missing POSITION in mesh '{}'", name);
      continue;
    }

    Mesh::MeshPrimitive mp{};
    mp.voffset = static_cast<uint32_t>(positions.size());
    mp.foffset = static_cast<uint32_t>(faces.size());
    mp.mat_idx = (pri.material >= 0) ? static_cast<uint32_t>(pri.material) : 0;

    // positions
    const tinygltf::Accessor& pos_acc = model.accessors[pos_it->second];
    const tinygltf::BufferView& pos_bv = model.bufferViews[pos_acc.bufferView];
    const tinygltf::Buffer& pos_buf = model.buffers[pos_bv.buffer];
    const int pos_stride = pos_acc.ByteStride(pos_bv);
    const unsigned char* pos_data =
        pos_buf.data.data() + pos_bv.byteOffset + pos_acc.byteOffset;

    for (size_t i = 0; i < pos_acc.count; ++i) {
      const float* v =
          reinterpret_cast<const float*>(pos_data + i * pos_stride);
      // glTF RUB → RDF: negate Y and Z
      positions.emplace_back(v[0], -v[1], -v[2]);
    }
    mp.vlength = static_cast<uint32_t>(pos_acc.count);

    // normals
    auto nrm_it = pri.attributes.find("NORMAL");
    if (nrm_it != pri.attributes.end()) {
      const tinygltf::Accessor& nrm_acc = model.accessors[nrm_it->second];
      const tinygltf::BufferView& nrm_bv =
          model.bufferViews[nrm_acc.bufferView];
      const tinygltf::Buffer& nrm_buf = model.buffers[nrm_bv.buffer];
      const int nrm_stride = nrm_acc.ByteStride(nrm_bv);
      const unsigned char* nrm_data =
          nrm_buf.data.data() + nrm_bv.byteOffset + nrm_acc.byteOffset;

      for (size_t i = 0; i < nrm_acc.count; ++i) {
        const float* n =
            reinterpret_cast<const float*>(nrm_data + i * nrm_stride);
        // glTF RUB → RDF: negate Y and Z
        normals.emplace_back(n[0], -n[1], -n[2]);
      }
    } else {
      // fill with zero normals to keep alignment with vbuff
      for (size_t i = 0; i < pos_acc.count; ++i) {
        normals.emplace_back(0.0f, 0.0f, 0.0f);
      }
    }

    // uvs (TEXCOORD_0)
    auto uv_it = pri.attributes.find("TEXCOORD_0");
    if (uv_it != pri.attributes.end()) {
      const tinygltf::Accessor& uv_acc = model.accessors[uv_it->second];
      const tinygltf::BufferView& uv_bv = model.bufferViews[uv_acc.bufferView];
      const tinygltf::Buffer& uv_buf = model.buffers[uv_bv.buffer];
      const int uv_stride = uv_acc.ByteStride(uv_bv);
      const unsigned char* uv_data =
          uv_buf.data.data() + uv_bv.byteOffset + uv_acc.byteOffset;

      for (size_t i = 0; i < uv_acc.count; ++i) {
        const float* u =
            reinterpret_cast<const float*>(uv_data + i * uv_stride);
        uvs.emplace_back(u[0], u[1]);
      }
    } else {
      // fill with zero UVs to keep alignment with vbuff
      for (size_t i = 0; i < pos_acc.count; ++i) {
        uvs.emplace_back(0.0f, 0.0f);
      }
    }

    // indices
    if (pri.indices >= 0) {
      const tinygltf::Accessor& idx_acc = model.accessors[pri.indices];
      const tinygltf::BufferView& idx_bv =
          model.bufferViews[idx_acc.bufferView];
      const tinygltf::Buffer& idx_buf = model.buffers[idx_bv.buffer];
      const unsigned char* idx_data =
          idx_buf.data.data() + idx_bv.byteOffset + idx_acc.byteOffset;

      std::vector<uint32_t> indices(idx_acc.count);
      switch (idx_acc.componentType) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
          for (size_t i = 0; i < idx_acc.count; ++i)
            indices[i] = idx_data[i];
          break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
          for (size_t i = 0; i < idx_acc.count; ++i)
            indices[i] = reinterpret_cast<const uint16_t*>(idx_data)[i];
          break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
          for (size_t i = 0; i < idx_acc.count; ++i)
            indices[i] = reinterpret_cast<const uint32_t*>(idx_data)[i];
          break;
        default:
          XEV_WARN("Unsupported index type {} in mesh '{}'",
                   idx_acc.componentType, name);
          continue;
      }

      for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        faces.emplace_back(indices[i], indices[i + 2], indices[i + 1]);
      }
    } else {
      for (uint32_t i = 0; i + 2 < mp.vlength; i += 3) {
        faces.emplace_back(i, i + 2, i + 1);
      }
    }

    mp.flength = static_cast<uint32_t>(faces.size()) - mp.foffset;
    primitives.push_back(mp);
  }

  mesh = Mesh(std::move(name), model_mat, std::move(primitives),
              std::move(positions), std::move(normals), std::move(uvs),
              std::move(faces));
}

void Scene::load_gltf(std::string_view filepath) {
  tinygltf::Model model;
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  std::string path(filepath);
  bool ret = false;

  if (path.length() >= 4 && path.substr(path.length() - 4) == ".glb") {
    ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);
  } else {
    ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
  }

  if (!warn.empty()) {
    XEV_WARN("glTF Warning: {}", warn);
  }
  if (!err.empty()) {
    XEV_ERROR("glTF Error: {}", err);
    return;
  }
  if (!ret) {
    XEV_ERROR("Failed to load glTF: {}", path);
    return;
  }
  if (is_loaded()) {
    XEV_ERROR("Loading glTF into an active scene. Please call .unload().");
    return;
  }

  // parsing geometry
  std::queue<std::pair<int, glm::mat4>> to_visit;
  for (const int& nidx : model.scenes[model.defaultScene].nodes) {
    to_visit.push({nidx, glm::mat4(1.0f)});
  }

  bool cam_found = false;
  while (!to_visit.empty()) {
    auto [node_idx, parent_mat] = to_visit.front();
    to_visit.pop();
    const tinygltf::Node& node = model.nodes[node_idx];

    XEV_INFO("Node name: {}", node.name);

    glm::mat4 local_mat(1.0f);
    if (node.matrix.size() == 16) {
      // glTF stores column-major, same as glm
      local_mat = glm::make_mat4(node.matrix.data());
    } else {
      glm::vec3 pos{0.0f};
      glm::quat rot{1.0f, 0.0f, 0.0f, 0.0f};
      glm::vec3 scale{1.0f};

      if (node.translation.size() == 3) {
        // glTF RUB → RDF: negate Y and Z
        pos = glm::vec3(node.translation[0], -node.translation[1],
                        -node.translation[2]);
      }
      if (node.rotation.size() == 4) {
        // glTF RUB → RDF: negate Y and Z of quaternion imaginary part
        rot = glm::quat(node.rotation[3], node.rotation[0], -node.rotation[1],
                        -node.rotation[2]);
      }
      if (node.scale.size() == 3) {
        scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
      }

      glm::mat4 T = glm::translate(glm::mat4(1.0f), pos);
      glm::mat4 R = glm::mat4_cast(rot);
      glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
      local_mat = T * R * S;
    }

    glm::mat4 abs_mat = parent_mat * local_mat;

    // parsing camera
    if (node.camera != -1 && node.name == "cam.active") {
      tinygltf::Camera camera = model.cameras[node.camera];
      XEV_ASSERT(camera.type == "perspective");
      // Extract position and rotation from the absolute matrix
      glm::vec3 abs_pos = glm::vec3(abs_mat[3]);
      glm::quat abs_rot = glm::quat_cast(abs_mat);
      m_active_cam =
          Camera(abs_rot, abs_pos,
                 glm::degrees(static_cast<float>(camera.perspective.yfov)));
      cam_found = true;
    }

    // parsing geometries
    if (node.mesh != -1) {
      Mesh mesh;
      parse_mesh(mesh, model, node, abs_mat);
      meshes.emplace_back(mesh);
    }

    for (const int& child : node.children) {
      to_visit.push({child, abs_mat});
    }
  }

  if (!cam_found) {
    XEV_WARN("No active camera found! Creating default fly cam.");
    m_active_cam = Camera();
    m_active_cam.pos = glm::vec3(0.0f, 0.0f, -5.0f);
  }
}

uint64_t Scene::size_device() const {
  uint64_t total_size = 0;
  for (const auto& mesh : meshes) {
    total_size += mesh.size_device();
  }
  return total_size;
}

uint64_t Scene::size_host() const {
  return sizeof(Camera);
}

bool Scene::is_loaded() const {
  if (meshes.size() == 0)
    return false;

  bool res = true;
  for (const auto& mesh : meshes) {
    res &= mesh.is_loaded();
  }
  return res;
}

void Scene::load(const Backend& backend) {
  for (auto& mesh : meshes) {
    if (!mesh.is_loaded())
      mesh.load(backend);
  }
}

void Scene::unload(const Backend& backend) {
  for (auto& mesh : meshes) {
    if (mesh.is_loaded())
      mesh.unload(backend);
  }
}

void Scene::create_test_triangle() {
  // TODO rewrite this
  XEV_INFO("Created test triangle.");
}

void Scene::destroy(const Backend& backend) {
  unload(backend);
  meshes.clear();
}

}  // namespace xev
