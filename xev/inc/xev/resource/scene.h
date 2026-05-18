#pragma once

#include <xev/camera.h>
#include <xev/resource/light.h>
#include <xev/resource/material.h>
#include <xev/resource/mesh.h>
#include <xev/resource/resource.h>
#include <xev/resource/texture.h>
#include <glm/glm.hpp>
#include <queue>
#include <string>
#include <string_view>
#include <vector>

namespace tinygltf {
struct Model;
struct Node;
struct Material;
struct Image;
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

  bool is_reserved() const override;
  void reserve(const Backend& backend) override;
  void release(const Backend& backend) override;

  Camera active_cam;
  std::vector<Mesh> meshes;
  std::vector<Material> materials;
  std::vector<Texture> textures;
  std::vector<Light> lights;

  void upload(const Backend& backend);
  void upload_meshes(const Backend& backend);
  void upload_textures(const Backend& backend);
  void upload_lights(const Backend& backend);

 private:
  void add_mesh(Mesh mesh);
  void parse_mesh(Mesh& mesh,
                  const tinygltf::Model& model,
                  const tinygltf::Node& node,
                  glm::mat4 model_mat) const;
  void parse_material(Material& material,
                      const tinygltf::Material& gltf_material) const;
  void parse_texture(Material& material,
                     const tinygltf::Material& gltf_material) const;

 public:
  struct SceneBuffer {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 view_pos;

    // ambient
    glm::vec3 ambient_color;
    float ambient_intensity;

    // fog
    glm::vec3 fog_color;
    float fog_density;

    // light
    VkDeviceAddress light_buffer_address;

    // material
    VkDeviceAddress material_buffer_address;
  };
  SceneBuffer scene_buffer;

  Buffer scene_buffer_device{VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                             VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT};
  Buffer light_buffer_device{VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                             VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT};
};

}  // namespace xev
