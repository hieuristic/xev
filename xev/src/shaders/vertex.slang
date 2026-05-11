module vertex;

export struct Vertex {
  float3 position;
  float3 normal;
  float2 uv;
};

export struct VertexIn {
  [[vk::location(0)]] float3 position : POSITION;
  [[vk::location(1)]] float3 normal : NORMAL;
  [[vk::location(2)]] float2 uv : TEXCOORD0;
};

export struct VertexOut {
  float4 position : SV_POSITION;
  float3 normal;
  float2 uv;
};

export struct VertexOutDebug {
  float4 position : SV_POSITION;
  float3 normal;
  float2 uv;
  uint face_id : PRIMITIVE_ID;
  float world_position;
};
