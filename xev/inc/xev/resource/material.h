#pragma once
#include <xev/color.h>
#include <xev/resource/resource.h>
#include <xev/resource/texture.h>
#include <string>

namespace xev {

class ResourceManager;

// in-sync with shaders/material.slang
struct MaterialGPU {
  Color4<uint8_t> albedo;
  float metal_coef;
  float rough_coef;
  float emiss_coef;

  uint32_t albedo_texid;
  uint32_t metallic_roughness_texid;
};

class Material : public Resource {
 public:
  std::string name;

  Color4<uint8_t> albedo{};
  float metal_coef = 0.0f;
  float rough_coef = 0.0f;
  float emiss_coef = 0.0f;

  uint32_t albedo_texid{0};
  uint32_t metallic_roughness_texid{0};

  uint64_t size_device() const override { return 0; }
  bool on_device() const override { return false; }

  void alloc(const ResourceManager& manager);
  void free(const ResourceManager& manager);
};

}  // namespace xev
