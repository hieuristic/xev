#language slang 2026

implementing mesh;

struct Scene {
  float4x4 view;
  float4x4 proj;
  float3 view_pos;

  // ambient
  float3 ambient_color;
  float ambient_intensity;

  // fog
  float3 fog_color;
  float fog_density;

  // light
  uint64_t light_buffer_address;

  // material
  uint64_t material_buffer_address;
};
