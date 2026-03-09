float hash11(uint n) {
  // integer hash copied from Hugo Elias
	n = (n << 13U) ^ n;
  n = n * (n * n * 15731U + 789221U) + 1376312589U;
  return float( n & uint(0x7fffffffU))/float(0x7fffffff);
}

struct PushConstants {
  column_major float4x4 mvp;
};

[[vk::push_constant]]
PushConstants pushConstants;

struct OmniLight {
  float3 pos;
  float intensity;
}

struct VertIn {
  [[vk::location(0)]] float3 position : POSITION;
  [[vk::location(1)]] float3 normal : NORMAL;
  [[vk::location(2)]] float2 uv : TEXCOORD0;
};

struct VertOut {
  float4 sv_position : SV_POSITION;
  float3 normal;
  float2 uv;
  uint primitiveID : PRIMITIVE_ID;
  float world_pos_h;
};

[shader("vertex")]
VertOut vert_main(VertIn vertex_input, uint vid: SV_VertexID) {
  VertOut output;
  output.sv_position = mul(pushConstants.mvp, float4(vertex_input.position, 1.0));
  output.normal = vertex_input.normal;
  output.uv = vertex_input.uv;
  output.primitiveID = vid / 3;
  output.world_pos_h = vertex_input.position.z;
  return output;
}

[shader("fragment")]
float4 frag_main(VertOut vertex_output): SV_TARGET {
  uint pid = vertex_output.primitiveID;
  // float r = hash11(pid);
  // float g = hash11(pid + 100);
  // float b = hash11(pid + 200);
  float r = vertex_output.normal.x;
  float g = vertex_output.normal.y;
  float b = vertex_output.normal.z;
  return float4(r, g, b, 1.0);
}
