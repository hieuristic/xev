#language slang 2026

implementing mesh;

struct Vertex {
  float3 position;
  float3 normal;
  float2 uv;
};

struct VODEBUG {
  float4 position : SV_POSITION;
  float3 normal;
  float2 uv;
  uint face_id : PRIMITIVE_ID;
  float3 world_xyz;
};
