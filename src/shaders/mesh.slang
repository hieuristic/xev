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

struct VertIn {
  [[vk::location(0)]] float3 position : POSITION;
};

struct VertOut {
  float4 sv_position : SV_POSITION;
  uint primitiveID : PRIMITIVE_ID;
  float world_pos_h;
};

[shader("vertex")]
VertOut vert_main(VertIn vertex_input, uint vid: SV_VertexID) {
  VertOut output;
  output.sv_position = mul(pushConstants.mvp, float4(vertex_input.position, 1.0));
  output.primitiveID = vid / 3;
  output.world_pos_h = vertex_input.position.z;
  return output;
}

[shader("fragment")]
float4 frag_main(VertOut vertex_output): SV_TARGET {
  uint pid = vertex_output.primitiveID;
  float r = hash11(pid);
  float g = hash11(pid + 100);
  float b = hash11(pid + 200);
  // float h = vertex_output.world_pos_h * 1.5 + 0.1;
  // float r, g, b;
  // r = g = b = h;
  return float4(r, g, b, 1.0);
}
