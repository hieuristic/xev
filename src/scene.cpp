#include <limits>
#include <xev/logger.h>
#include <xev/scene.h>

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
  m_meshes.clear();

  // parsing geometry
  std::queue<int> to_visit;
  for (const int &nidx : model.scenes[model.defaultScene].nodes) {
    to_visit.push(nidx);
  }

  bool cam_found = false;
  while (!to_visit.empty()) {
    int node_idx = to_visit.front();
    const tinygltf::Node &node = model.nodes[node_idx];

    XEV_INFO("Node name: {}", node.name);

    // parsing camera
    if (node.camera != -1 && node.name == "cam.active") {
      tinygltf::Camera camera = model.cameras[node.camera];
      XEV_INFO("node matrix size: {}", node.matrix.size());
      XEV_ASSERT(camera.type == "perspective");
      glm::quat rot;
      if (node.rotation.size() == 4) {
        // glTF RUB → RDF: negate Y and Z of quaternion imaginary part
        rot = glm::quat(node.rotation[3], node.rotation[0],
                        -node.rotation[1], -node.rotation[2]);
      }
      glm::vec3 pos;
      if (node.translation.size() == 3) {
        // glTF RUB → RDF: negate Y and Z
        pos = glm::vec3(node.translation[0], -node.translation[1],
                        -node.translation[2]);
      }
      m_active_cam = Camera(rot, pos, glm::degrees(static_cast<float>(camera.perspective.yfov)));
      cam_found = true;
    }

    // parsing geometries
    if (node.mesh != -1) {
      m_meshes.emplace_back(Mesh(model, node, m_vert_buffer, m_face_buffer));
    }

    // TODO: bfs traverse, assume no hierachy for now.
    // for (const int &child : children) {
    //   to_visit.push(child);
    // }
    to_visit.pop();
  }

  if (!cam_found) {
    XEV_ERROR("No active camera found!");
  }
}

void Scene::create_test_triangle() {
  // TODO rewrite this
  XEV_INFO("Created test triangle.");
}

} // namespace xev
