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

  uint32_t vertex_offset = 0;

  // parsing camera
  XEV_INFO("NUMBER OF CAMERAS: {}", model.cameras.size());
  for (const tinygltf::Camera &cam : model.cameras) {
    XEV_INFO("CAMERA NAME: {}", cam.name);
  }

  // parsing geometry
  tinygltf::Scene raw_scene = model.scenes[model.defaultScene];

  std::queue<int> to_visit;
  for (const int &nidx : raw_scene.nodes) {
    to_visit.push(nidx);
  }

  XEV_INFO("NUMBER OF NODES: {}", to_visit.size());
  bool cam_found = false;
  while (!to_visit.empty()) {
    int node_idx = to_visit.front();
    tinygltf::Node node = model.nodes[node_idx];

    if (node.camera != -1 && node.name.compare("cam.active")) {
      tinygltf::Camera camera = model.cameras[node.camera];
      assert (camera.type.compare("perspective"));
      assert (node.rotation.size() == 4);
      assert (node.translation.size() == 3);
      Scene.m_active_cam = Camera(node.rotation, node.translation, camera.perspective.yfov);
      cam_found = true;
    }

    if (node.mesh != -1) {
      std::unique<Mesh> mesh = std::make_unique<Mesh>();
      mesh->load_from_gltf();
      Scene.m_meshes.push_back(std::move(mesh));
    }

    for (const int& child : children) {
      to_visit.push(child);
    }
    to_visit.pop();
  }
  XEV_ERROR_IF_FAILED(cam_found, "No active camera found!");
}

void Scene::create_test_triangle() {
  // TODO rewrite this
  XEV_INFO("Created test triangle.");
}

} // namespace xev
