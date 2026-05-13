#define DEBUG_MESH

import vertex;
import light;

struct PushConstants {
  column_major float4x4 mvp;
  uint64_t scene_uniform;
  uint64_t vertex_buffer;
  uint32_t material_id;
  uint32_t padding;
};

[[vk::push_constant]]
PushConstants pushConstants;

struct VertIn {
  [[vk::location(0)]] float3 position : POSITION;
  [[vk::location(1)]] float3 normal : NORMAL;
  [[vk::location(2)]] float2 uv : TEXCOORD0;
};

struct VertOut {
  float4 position : SV_POSITION;
  float3 normal;
  float2 uv;
  uint face_id : PRIMITIVE_ID;
  float world_pos_h;
};

[shader("vertex")]
VertOut main(VertIn vertex_input) {
  VertOut output;
  output.position = mul(pushConstants.mvp, float4(vertex_input.position, 1.0));
  // output.position = float4(vertex_input.position.xy, 0.5, 1.0);
  output.normal = vertex_input.normal;
  output.uv = vertex_input.uv;
  return output;
}

[shader("fragment")]
float4 main(VertOut vertex_output): SV_TARGET {
  float r = vertex_output.normal.x * 0.5 + 0.5;
  float g = vertex_output.normal.y * 0.5 + 0.5;
  float b = vertex_output.normal.z * 0.5 + 0.5;
  // return float4(1.0, 1.0, 1.0, 1.0);
  return float4(r, g, b, 1.0);
}
