#language slang 2026

implementing mesh;

struct Scene {
  float4x4 view;
  float4x4 proj;
  float3 view_xyz;

  // ambient
  float3 ambient_color;
  float ambient_intensity;

  // fog
  float3 fog_color;
  float fog_density;

  // light
  Light* lights;
  uint32_t num_lights;

  // material
  Material* materials;
};
