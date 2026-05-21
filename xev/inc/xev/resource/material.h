#pragma once
#include <xev/resource/resource.h>
#include <xev/resource/texture.h>
#include <xev/color.h>
#include <string>

namespace xev {

class Material : Resource {
 public:
  std::string name;

  Color4 base_color{};
  float metal_coef = 0.0f;
  float rough_coef = 0.0f;
  float emiss_coef = 0.0f;

  uint32_t diffuse_texid{0};
  uint32_t metallic_roughness_texid{0};

  uint64_t size_device() const override;
  bool is_reserved() const override { return false; }
  void reserve(const Backend& backend) override {}
  void release(const Backend& backend) override {}
};

class MaterialHighP {
 public:
  std::string name;

  Color4HighP base_color{};
  double metal_coef = 0.0f;
  double rough_coef = 0.0f;
  double emiss_coef = 0.0f;

  uint32_t color_texid{0};
  uint32_t metal_texid{0};
  uint32_t rough_texid{0};
  uint32_t emiss_texid{0};
};

}  // namespace xev
