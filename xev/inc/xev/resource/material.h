#pragma once
#include <xev/resource/resource.h>
#include <xev/color.h>
#include <string>

namespace xev {

class ResourceManager;

template <typename T = float>
class Material : public Resource {
 public:
  std::string name;

  Color4<T> base_color{};
  T metal_coef = 0.0f;
  T rough_coef = 0.0f;
  T emiss_coef = 0.0f;

  uint32_t diffuse_texid{0};
  uint32_t metallic_roughness_texid{0};

  uint64_t size_device() const override { return 0; }
  bool on_device() const override { return false; }
  void alloc(const ResourceManager& manager) {}
  void free(const ResourceManager& manager) {}
};

}  // namespace xev
