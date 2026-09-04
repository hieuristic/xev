#include <xev/filesystem/fs.h>
#include <xev/logger.h>
#include <xev/resource/image.h>
#include <xev/resource/scene.h>
#include <xev/resource_manager.h>

#include <tiny_gltf.h>
#include <filesystem>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <limits>

namespace xev {

Scene::Scene() {}

void Scene::parse_mesh(std::vector<Mesh>& meshes,
                       const tinygltf::Model& model,
                       const tinygltf::Node& node,
                       glm::mat4 model_mat) const {
  std::string name = node.name;

  const tinygltf::Mesh& gltf_mesh = model.meshes[node.mesh];

  for (const tinygltf::Primitive& pri : gltf_mesh.primitives) {
    if (pri.mode != -1 && pri.mode != TINYGLTF_MODE_TRIANGLES) {
      XEV_WARN("Skiipping non-triangle primitive in mesh '{}'", name);
      continue;
    }

    auto pos_it = pri.attributes.find("POSITION");
    if (pos_it == pri.attributes.end()) {
      XEV_WARN("Primitive missing POSITION in mesh '{}'", name);
      continue;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<glm::uvec3> faces;
    uint32_t mat_id =
        (pri.material >= 0) ? static_cast<uint32_t>(pri.material) : 0;

    {  // positions
      const tinygltf::Accessor& pos_acc = model.accessors[pos_it->second];
      const tinygltf::BufferView& pos_bv =
          model.bufferViews[pos_acc.bufferView];
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
    }

    {  // normals
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
        for (size_t i = 0; i < positions.size(); ++i) {
          normals.emplace_back(0.0f, 0.0f, 0.0f);
        }
      }
    }

    {  // TEXCOORD_0

      auto uv_it = pri.attributes.find("TEXCOORD_0");
      if (uv_it != pri.attributes.end()) {
        const tinygltf::Accessor& uv_acc = model.accessors[uv_it->second];
        const tinygltf::BufferView& uv_bv =
            model.bufferViews[uv_acc.bufferView];
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
        for (size_t i = 0; i < positions.size(); ++i) {
          uvs.emplace_back(0.0f, 0.0f);
        }
      }
    }

    {  // faces
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
        for (uint32_t i = 0; i + 2 < positions.size(); i += 3) {
          faces.emplace_back(i, i + 2, i + 1);
        }
      }
    }

    meshes.push_back(Mesh(name, model_mat, mat_id, std::move(positions),
                          std::move(normals), std::move(uvs),
                          std::move(faces)));
  }
}

void Scene::load_gltf(const FileSystem& fileSys,
                      std::string_view filepath,
                      uint32_t idxOffset_) {
  idxOffset = idxOffset_;

  tinygltf::Model model;
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  std::vector<uint8_t> gltfData = fileSys.read(filepath);
  fileSys.attach(loader);

  bool ret = false;
  if (filepath.length() >= 4 &&
      filepath.substr(filepath.length() - 4) == ".glb") {
    ret = loader.LoadBinaryFromMemory(
        &model, &err, &warn, gltfData.data(),
        static_cast<unsigned int>(gltfData.size()), "");
  } else {
    ret = loader.LoadASCIIFromString(
        &model, &err, &warn, reinterpret_cast<const char*>(gltfData.data()),
        static_cast<unsigned int>(gltfData.size()), "");
  }

  if (!warn.empty()) XEV_WARN("glTF Warning: {}", warn);
  if (!err.empty()) XEV_ERROR("glTF Error: {}", err);
  if (!ret) return;

  if (on_device()) {
    XEV_ERROR("Loading glTF into an active scene. Please call .release().");
    return;
  }

  // parse light
  for (const auto& gltf_light : model.lights) {
    Light light = {
        .name = gltf_light.name,
        .color =
            {
                static_cast<float>(gltf_light.color[0]),
                static_cast<float>(gltf_light.color[1]),
                static_cast<float>(gltf_light.color[2]),
            },
        .range = static_cast<float>(gltf_light.range),
        .intensity = static_cast<float>(gltf_light.intensity),
        // .type = TODO
    };
    lights.emplace_back(light);
  };

  // parsing material
  for (const auto& gltf_mat : model.materials) {
    Material mat;
    mat.name = gltf_mat.name;
    mat.albedo = Color4<uint8_t>(gltf_mat.pbrMetallicRoughness.baseColorFactor);
    mat.metal_coef =
        static_cast<float>(gltf_mat.pbrMetallicRoughness.metallicFactor);
    mat.rough_coef =
        static_cast<float>(gltf_mat.pbrMetallicRoughness.roughnessFactor);

    int albedo_idx = gltf_mat.pbrMetallicRoughness.baseColorTexture.index;
    mat.albedo_texid =
        (albedo_idx >= 0) ? model.textures[albedo_idx].source : 0xFFFFFFFF;

    int mr_idx = gltf_mat.pbrMetallicRoughness.metallicRoughnessTexture.index;
    mat.metallic_roughness_texid =
        (mr_idx >= 0) ? model.textures[mr_idx].source : 0xFFFFFFFF;
    materials.emplace_back(mat);
  }

  // parsing images
  for (const auto& gltf_img : model.images) {
    if (gltf_img.bits != 8) XEV_INFO("LOADING NON-8BITS IMAGES");
    XEV_INFO("TEXTURE: {}x{}x{} {}-bits", gltf_img.width, gltf_img.height,
             gltf_img.component, gltf_img.bits);
    Image img{static_cast<uint32_t>(gltf_img.width),
              static_cast<uint32_t>(gltf_img.height), VK_FORMAT_R8G8B8A8_SRGB,
              VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT};

    if (gltf_img.component == 3) {
      img.host_data.resize(gltf_img.width * gltf_img.height * 4);
      for (size_t i = 0; i < gltf_img.width * gltf_img.height; ++i) {
        img.host_data[i * 4 + 0] = gltf_img.image[i * 3 + 0];  // R
        img.host_data[i * 4 + 1] = gltf_img.image[i * 3 + 1];  // G
        img.host_data[i * 4 + 2] = gltf_img.image[i * 3 + 2];  // B
        img.host_data[i * 4 + 3] = 255;                        // Alpha
      }
    } else if (gltf_img.component == 4) {
      std::copy(gltf_img.image.begin(), gltf_img.image.end(),
                std::back_inserter(img.host_data));
    } else {
      XEV_WARN("Unsupported channel count: {}", gltf_img.component);
    }
    images.emplace_back(img);
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

    // parse camera
    if (node.camera != -1 && node.name == "cam.active") {
      tinygltf::Camera camera = model.cameras[node.camera];
      XEV_ASSERT(camera.type == "perspective");
      // Extract position and rotation from the absolute matrix
      glm::vec3 abs_pos = glm::vec3(abs_mat[3]);
      glm::quat abs_rot = glm::quat_cast(abs_mat);
      XEV_INFO("Camera yfov {}", camera.perspective.yfov);
      // active_cam =
      //     Camera(abs_rot, abs_pos,
      //            glm::degrees(static_cast<float>(camera.perspective.yfov)));
      active_cam = Camera(abs_rot, abs_pos,
                          70);  // TODO REMOVE HARDCODE LATER, THIS IS FOR DEBUG
      XEV_INFO("Camera zfar {}, znear {}", active_cam.far, active_cam.near);
      cam_found = true;
    }

    // parse geometries
    if (node.mesh != -1) {
      parse_mesh(meshes, model, node, abs_mat);
    }

    // parse light
    if (node.light != -1) {
      lights[node.light].position = glm::vec3(abs_mat[3]);
      lights[node.light].direction = glm::normalize(glm::vec3(abs_mat[2]));
    }

    for (const int& child : node.children) {
      to_visit.push({child, abs_mat});
    }
  }

  if (!cam_found) {
    XEV_WARN("No active camera found! Creating default fly cam.");
    active_cam = Camera();
    active_cam.pos = glm::vec3(0.0f, 0.0f, -5.0f);
  }
}  // namespace xev

uint64_t Scene::size_device() const {
  uint64_t total_size = 0;
  for (const auto& mesh : meshes) {
    total_size += mesh.size_device();
  }
  return total_size;
}

bool Scene::on_device() const {
  if (meshes.size() == 0) return false;

  bool res = true;
  for (const auto& mesh : meshes) {
    res &= mesh.on_device();
  }
  return res;
}

void Scene::alloc(const ResourceManager& manager) {
  scene_device.size = sizeof(SceneBuffer);
  manager.alloc(scene_device);

  if (!lights.empty()) {
    lights_device.size = sizeof(LightGPU) * lights.size();
    manager.alloc(lights_device);
  }

  if (!materials.empty()) {
    materials_device.size = sizeof(MaterialGPU) * materials.size();
    manager.alloc(materials_device);
  }

  for (auto& tex : images) {
    if (!tex.on_device()) manager.alloc(tex);
  }
  for (auto& mesh : meshes) {
    if (!mesh.on_device()) mesh.alloc(manager);
  }
}

void Scene::free(const ResourceManager& manager) {
  manager.free(scene_device);
  manager.free(lights_device);
  manager.free(materials_device);

  for (auto& tex : images) {
    if (tex.on_device()) manager.free(tex);
  }

  for (auto& mesh : meshes) {
    if (mesh.on_device()) mesh.free(manager);
  }
}

void Scene::upload_images(const ResourceManager& manager,
                          const HotExec& hot_exec) {
  std::vector<Buffer> stagings;
  for (auto& img : images) {
    uint64_t size = img.width * img.height * 4;
    Buffer staging{
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_AUTO,
    };
    manager.alloc(staging);

    void* map_ = staging.alloc_info.pMappedData;
    memcpy(map_, img.host_data.data(), size);

    stagings.emplace_back(staging);
  }

  hot_exec.run([&](const VkCommandBuffer cmdbuf) {
    for (uint32_t i = 0; i < images.size(); i++) {
      auto& img = images[i];
      img.layout = VK_IMAGE_LAYOUT_UNDEFINED;
      img.update_layout(cmdbuf, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

      VkBufferImageCopy reg = {
          .bufferOffset = 0,
          .bufferRowLength = 0,
          .bufferImageHeight = 0,
          .imageSubresource =
              {
                  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                  .mipLevel = 0,
                  .baseArrayLayer = 0,
                  .layerCount = 1,
              },
          .imageOffset = {0, 0, 0},
          .imageExtent =
              {
                  static_cast<uint32_t>(img.width),
                  static_cast<uint32_t>(img.height),
                  1,
              },
      };
      vkCmdCopyBufferToImage(cmdbuf, stagings[i].buffer, img.image,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &reg);

      img.update_layout(cmdbuf, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
  });

  for (auto& staging : stagings)
    manager.free(staging);
}

void Scene::upload_meshes(const ResourceManager& manager,
                          const HotExec& hot_exec) {
  for (auto& mesh : meshes)
    mesh.upload(manager, hot_exec);
}

void Scene::upload_lights(const ResourceManager& manager,
                          const HotExec& hot_exec) {
  if (lights.empty()) return;

  uint64_t size = lights.size() * sizeof(LightGPU);
  Buffer staging{size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO};
  manager.alloc(staging);

  XEV_INFO("UPLOADING {} LIGHT(S)", lights.size());
  {
    LightGPU* map_ = static_cast<LightGPU*>(staging.alloc_info.pMappedData);
    // uint64_t offset_ = 0;
    for (uint32_t i = 0; i < lights.size(); i++) {
      map_[i] = {
          .type = static_cast<uint32_t>(lights[i].type),
          .position = lights[i].position,
          .direction = lights[i].direction,
          .color = lights[i].color,
          .range = lights[i].range,
          .intensity = lights[i].intensity,
          .offset = lights[i].offset,
          .shadowmap_id = lights[i].shadowmap_id,
      };
      // memcpy((char*)map_ + offset_, &lights[i], sizeof(Light));
      // offset_ += sizeof(Light);
    }
  }

  lights_device.size = size;
  hot_exec.run([&](const VkCommandBuffer cmdbuf) {
    const VkBufferCopy reg = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size,
    };
    vkCmdCopyBuffer(cmdbuf, staging.buffer, lights_device.buffer, 1, &reg);
  });

  manager.free(staging);
}

void Scene::upload_materials(const ResourceManager& manager,
                             const HotExec& hot_exec) {
  if (materials.empty()) return;

  uint64_t size = materials.size() * sizeof(MaterialGPU);
  Buffer staging{size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO};
  manager.alloc(staging);

  {
    MaterialGPU* map_ =
        static_cast<MaterialGPU*>(staging.alloc_info.pMappedData);
    uint64_t offset_ = 0;
    for (uint32_t i = 0; i < materials.size(); i++) {
      uint32_t albedo_id = (materials[i].albedo_texid != 0xFFFFFFFF)
                               ? (materials[i].albedo_texid + idxOffset)
                               : 0xFFFFFFFF;
      uint32_t mr_id = (materials[i].metallic_roughness_texid != 0xFFFFFFFF)
                           ? (materials[i].metallic_roughness_texid + idxOffset)
                           : 0xFFFFFFFF;
      map_[i] = {
          .albedo = materials[i].albedo,
          .metal_coef = materials[i].metal_coef,
          .rough_coef = materials[i].rough_coef,
          .emiss_coef = materials[i].emiss_coef,
          .albedo_texid = albedo_id,
          .metallic_roughness_texid = mr_id,
      };
      // memcpy((char*)map_ + offset_, (void*)&materials[i], sizeof(Material));
      // offset_ += sizeof(Material);
    }
  }

  materials_device.size = size;
  hot_exec.run([&](const VkCommandBuffer cmdbuf) {
    const VkBufferCopy reg = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size,
    };
    vkCmdCopyBuffer(cmdbuf, staging.buffer, materials_device.buffer, 1, &reg);
  });

  manager.free(staging);
}

void Scene::upload_scene(const ResourceManager& manager,
                         const HotExec& hot_exec) {
  uint64_t size = sizeof(SceneBuffer);
  Buffer staging{size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO};
  manager.alloc(staging);

  {
    void* map_ = staging.alloc_info.pMappedData;
    memcpy(map_, &scene_buffer, size);
  }

  scene_device.size = size;
  hot_exec.run([&](const VkCommandBuffer cmdbuf) {
    const VkBufferCopy reg = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size,
    };
    vkCmdCopyBuffer(cmdbuf, staging.buffer, scene_device.buffer, 1, &reg);
  });

  manager.free(staging);
}

void Scene::upload(const ResourceManager& manager, const HotExec& hot_exec) {
  upload_meshes(manager, hot_exec);
  upload_images(manager, hot_exec);
  upload_lights(manager, hot_exec);
  upload_materials(manager, hot_exec);

  scene_buffer.exposure = active_cam.exposure();
  scene_buffer.light_buffer_address = lights_device.addr;
  scene_buffer.num_lights = lights.size();
  scene_buffer.material_buffer_address = materials_device.addr;
  upload_scene(manager, hot_exec);
}

void Scene::bind(const GlobalDescriptorSet& desc_set) {
  for (uint32_t i = 0; i < images.size(); i++) {
    desc_set.set(images[i], i + idxOffset);
    XEV_INFO("ADDING IMAGE {} TO DESCRIPTOR SET", i);
  }
}

void Scene::bind(const GlobalDescriptorSet& desc_set, uint32_t startIdx) {
  for (uint32_t i = 0; i < images.size(); i++) {
    desc_set.set(images[i], i + startIdx);
    XEV_INFO("ADDING IMAGE {} TO DESCRIPTOR SET", i);
  }
}

void Scene::create_test_triangle() {
  XEV_INFO("Created test triangle.");
  XEV_ASSERT(meshes.size() == 0);

  {
    glm::mat4 model_mat = glm::mat4(1.0);
    std::vector<glm::vec3> p = {
        glm::vec3(0.0f, 0.5f, 1.0f),
        glm::vec3(-0.5f, -0.5f, 1.0f),
        glm::vec3(0.5f, -0.5f, 1.0f),
    };
    std::vector<glm::vec3> n = {
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
    };
    std::vector<glm::vec2> u = {
        glm::vec2(0.0f, 0.5f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
    };
    std::vector<glm::uvec3> f = {
        glm::uvec3(0, 1, 2),
    };
    uint32_t mat_id = 0;
    Mesh trichen("trichen", model_mat, mat_id, p, n, u, f);
    meshes.emplace_back(trichen);
  }

  {
    glm::quat r = glm::quat();
    glm::vec3 p = glm::vec3();
    active_cam = Camera(r, p, 90.0f);
  }
}

void Scene::destroy(const ResourceManager& manager) {
  free(manager);
  meshes.clear();
}

void Scene::save_bin(std::filesystem::path& outFile) {
  XEV_ASSERT(!std::filesystem::exists(outFile), "Writing to an existing file!");
  std::ofstream out(outFile, std::ios::binary);
  XEV_ASSERT(out.is_open());

  out.write(reinterpret_cast<const char*>(&active_cam), sizeof(active_cam));
  out.write(reinterpret_cast<const char*>(&scene_buffer), sizeof(scene_buffer));
  for (auto& mesh : meshes)
    mesh.write(out);
}

}  // namespace xev
