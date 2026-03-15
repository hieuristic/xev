module scene;

import light;
import material;

export struct Scene {
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
  uint32_t light_ids[];

  // material
  uint32_t material_ids[];
};
