static float2 positions[3] = {
  float2(0.0, -0.5),
  float2(0.5, 0.5),
  float2(-0.5, 0.5)
};

static float3 colors[3] = {
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0)
};

struct VertOut {
  float3 color;
  float4 sv_position : SV_POSITION;
};

[shader("vertex")]
VertOut vert_main(uint vid: SV_VertexID) {
  VertOut output;
  output.sv_position = float4(positions[vid], 0.0, 1.0);
  output.color = colors[vid];
  return output;
}

[shader("fragment")]
float4 frag_main(VertOut vertex_output): SV_TARGET {
  return float4(vertex_output.color, 0.0);
}
