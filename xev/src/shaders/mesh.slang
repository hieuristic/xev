#language slang 2026
#define DEBUG_MESH

module mesh;
__include vertex;
__include light;
__include scene;
__include push_const;
__include material;

[shader("vertex")]
VODEBUG main(uint vid : SV_VertexID) {
  VODEBUG vo;
  Vertex vi = pc.vert_ptr[vid];
  vo.position = mul(pc.view_proj, float4(vi.position, 1.0));
  vo.normal = vi.normal;
  vo.uv = vi.uv;
  vo.face_id = vid / 3;
  vo.world_position = 0.0;
  return vo;
}

[shader("fragment")]
float4 main(VODEBUG vo): SV_TARGET {
  float r = vo.normal.x * 0.5 + 0.5;
  float g = vo.normal.y * 0.5 + 0.5;
  float b = vo.normal.z * 0.5 + 0.5;
  // return float4(1.0, 1.0, 1.0, 1.0);
  return float4(r, g, b, 1.0);
}
