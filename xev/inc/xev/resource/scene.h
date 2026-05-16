#pragma once

#include <xev/camera.h>
#include <xev/resource/material.h>
#include <xev/resource/mesh.h>
#include <xev/resource/resource.h>
#include <glm/glm.hpp>
#include <queue>
#include <string>
#include <string_view>
#include <vector>

namespace tinygltf {
struct Model;
struct Node;
struct Material;
}  // namespace tinygltf

namespace xev {

class Backend;

class Scene : Resource {
 public:
  Scene();
  void destroy(const Backend& backend);

  void load_gltf(std::string_view filepath);
  void create_test_triangle();

  uint64_t size_device() const override;
  uint64_t size_host() const override;

  bool is_loaded() const override;
  void load(const Backend& backend) override;
  void unload(const Backend& backend) override;

  Camera active_cam;
  std::vector<Mesh> meshes;
  std::vector<Material> materials;

 private:
  void add_mesh(Mesh mesh);
  void parse_mesh(Mesh& mesh,
                  const tinygltf::Model& model,
                  const tinygltf::Node& node,
                  glm::mat4 model_mat) const;
  void parse_material(Material& material,
                      const tinygltf::Material& gltf_material) const;
};

}  // namespace xev
