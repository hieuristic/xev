#pragma once

#include <xev/camera.h>
#include <xev/global_descriptor_set.h>
#include <xev/resource/image.h>
#include <xev/resource/light.h>
#include <xev/resource/material.h>
#include <xev/resource/mesh.h>
#include <xev/resource/resource.h>
#include <xev/resource/sampler.h>
#include <xev/scene_buffer.h>
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

struct Backend;
struct FileSystem;

struct Scene : public Resource {
  Scene();
  void destroy(const ResourceManager& manager);

  void load_gltf(const FileSystem& fileSys, std::string_view filepath);
  void create_test_triangle();

  uint64_t size_device() const override;
  bool on_device() const override;

  void alloc(const ResourceManager& manager);
  void free(const ResourceManager& manager);

  Camera active_cam;
  SceneBuffer scene_buffer;
  std::vector<Mesh> meshes;
  std::vector<Image> images;
  std::vector<Light> lights;
  std::vector<Material> materials;

  std::vector<Camera> cameras;

  Buffer scene_device{VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                      VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                      VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT};
  Buffer lights_device{VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                       VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                       VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT};
  Buffer materials_device{VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                          VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT};

  void upload(const ResourceManager& manager, const HotExec& hot_exec);
  void upload_meshes(const ResourceManager& manager, const HotExec& hot_exec);
  void upload_images(const ResourceManager& manager, const HotExec& hot_exec);
  void upload_lights(const ResourceManager& manager, const HotExec& hot_exec);
  void upload_materials(const ResourceManager& manager,
                        const HotExec& hot_exec);
  void upload_scene(const ResourceManager& manager, const HotExec& hot_exec);

  void bind(const GlobalDescriptorSet& desc_set);

 private:
  void parse_mesh(std::vector<Mesh>& out_meshes,
                  const tinygltf::Model& model,
                  const tinygltf::Node& node,
                  glm::mat4 model_mat) const;
};

}  // namespace xev
