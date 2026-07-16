#language slang 2026

implementing mesh;

struct Material {
  uint8_t4 albedo;
  float metal_coef;
  float rough_coef;
  float emiss_coef;

  uint32_t albedo_texid;
  uint32_t metallic_roughness_texid;
};
