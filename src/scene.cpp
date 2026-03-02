#include <xev/scene.h>
#include <xev/logger.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

namespace xev {

Scene::Scene() {}

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
  }
  if (!ret) {
    XEV_ERROR("Failed to load glTF: {}", path);
    return;
  }

  m_vert_buffer.clear();
  m_face_buffer.clear();

  // Parse vertices and indices
  for (const auto& mesh : model.meshes) {
    for (const auto& primitive : mesh.primitives) {

      // Extract positions
      auto it = primitive.attributes.find("POSITION");
      if (it != primitive.attributes.end()) {
        const tinygltf::Accessor& accessor = model.accessors[it->second];
        const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

        const float* positions = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

        size_t byteStride = accessor.ByteStride(bufferView);
        if (byteStride == 0) byteStride = sizeof(float) * 3;

        for (size_t i = 0; i < accessor.count; ++i) {
          const float* p = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(positions) + i * byteStride);
          m_vert_buffer.push_back(glm::vec3(p[0], p[1], p[2]));
        }
      }

      // Extract indices
      if (primitive.indices >= 0) {
        const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
        const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

        size_t byteStride = accessor.ByteStride(bufferView);
        const uint8_t* data = &buffer.data[bufferView.byteOffset + accessor.byteOffset];

        for (size_t i = 0; i < accessor.count; i += 3) {
          uint32_t indices[3] = {0, 0, 0};
          for (int j = 0; j < 3; ++j) {
            if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
              const uint16_t* p = reinterpret_cast<const uint16_t*>(data + (i + j) * (byteStride == 0 ? 2 : byteStride));
              indices[j] = *p;
            } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
              const uint32_t* p = reinterpret_cast<const uint32_t*>(data + (i + j) * (byteStride == 0 ? 4 : byteStride));
              indices[j] = *p;
            } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
               const uint8_t* p = reinterpret_cast<const uint8_t*>(data + (i + j) * (byteStride == 0 ? 1 : byteStride));
               indices[j] = *p;
            }
          }
          m_face_buffer.push_back(glm::uvec3(indices[0], indices[1], indices[2]));
        }
      }
    }
  }

  // Parse active camera
  if (!model.cameras.empty()) {
    const tinygltf::Camera& gltf_cam = model.cameras[0];
    if (gltf_cam.type == "perspective") {
      m_active_cam = Camera(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f),
                            glm::degrees(gltf_cam.perspective.yfov));
      m_active_cam.near = gltf_cam.perspective.znear;
      m_active_cam.far = gltf_cam.perspective.zfar > 0.0f ? gltf_cam.perspective.zfar : 1000.0f;
    } else {
       m_active_cam = Camera();
    }

    // Look for node transform if camera is attached
    for (const auto& node : model.nodes) {
        if (node.camera == 0) {
            if (node.translation.size() == 3) {
                m_active_cam.pos = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
            }
            if (node.rotation.size() == 4) {
                m_active_cam.rot = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
            }
            break;
        }
    }
  } else {
    m_active_cam = Camera();
  }

  XEV_INFO("Loaded glTF: {} ({} vertices, {} faces)", path, m_vert_buffer.size(), m_face_buffer.size());
}

} // namespace xev
