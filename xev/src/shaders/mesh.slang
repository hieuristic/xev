#language slang 2026
#define DEBUG_MESH_NORMAL
// #define DEBUG_MESH_DEPTH

module mesh;
__include vertex;
__include light;
__include scene;
__include push_const;
__include material;
__include global_desc;

import pbr;

[shader("vertex")]
VODEBUG main(uint vid : SV_VertexID) {
  VODEBUG vo;
  Vertex vi = pc.vert_ptr[vid];
  vo.position = mul(pc.view_proj, float4(vi.position, 1.0));
  vo.normal = vi.normal;
  vo.uv = vi.uv;
  vo.face_id = vid / 3;
  vo.world_xyz = vi.position;
  return vo;
}

[shader("fragment")]
float4 main(VODEBUG vo): SV_TARGET {
#ifdef DEBUG_MESH_NORMAL
  float r = vo.normal.x * 0.5 + 0.5;
  float g = vo.normal.y * 0.5 + 0.5;
  float b = vo.normal.z * 0.5 + 0.5;
  return float4(r, g, b, 1.0);
#else
  Material mat = pc.scene_ptr->materials[pc.mat_id];
  float3 diffuse = sample2DLinear(mat.albedo_texid, vo.uv).rgb;
  float4 metallic_roughness = sample2DLinear(mat.metallic_roughness_texid, vo.uv);
  float roughness = metallic_roughness.g * mat.rough_coef;
  float metallic = metallic_roughness.b * mat.metal_coef;
  float3 f0 = float3(0.0);
  float3 cam_xyz = pc.scene_ptr.view_xyz;
  float3 pix_xyz = vo.world_xyz;

  float3 v = normalize(cam_xyz - pix_xyz);
  float3 n = vo.normal;
  float3 color = float3(0.0);
  // TODO: iterate over lights here
  for (int i = 0; i < pc->scene_ptr->num_lights; i++) {
    Light light = pc->scene_ptr->lights[i];
    float3 l = normalize(light.xyz - pix_xyz); // for light
    float3 h = normalize(v + l);
    float dist = length(light.xyz - pix_xyz);
    float3 radiance = light.color * light.intensity / (dist * dist) * 1000;
    color += brdf(diffuse, roughness, metallic, f0, v, n, l, h) *
             radiance * max(dot(n, l), 0.0);
  }
  return float4(color, 1.0);
#endif
}
