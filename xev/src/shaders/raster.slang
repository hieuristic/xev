#language slang 2026
// #define DEBUG_FONT_ATLAS
// #define DEBUG_GUI

module raster;
import global_desc;

struct DrawInfo {
  float4x4 transform;
  float4 uvBounds; // left top right bottom
  float3 color;
  uint32_t texID;
  uint32_t isMSDF;
};

struct PushConstants {
  DrawInfo* infoBuffer;
};

[[vk::push_constant]]
uniform PushConstants pc;

struct VertexIn {
  uint32_t vertID : SV_VertexID;
  uint32_t instID : SV_InstanceID;
};

struct VertexOut {
  uint32_t instID : SV_InstanceID;
  float4 pos  : SV_Position;
  float2 uv   : TEXCOORD0;
  float2 uv1  : TEXCOORD1;
};

static const float2 billboardCorners[3] = {
    float2(-1.0, -1.0),
    float2( 3.0, -1.0),
    float2(-1.0,  3.0)
};

[shader("vertex")]
VertexOut main(VertexIn vi) {
  // constructing a isoscles right triangle with right angle at top right
  // (-1,-1) ---- (-1,3)
  //   | draw |  /
  //   | area  /
  //   | - - /
  //   |   /
  // (3,-1)
  float2 pos = billboardCorners[vi.vertID];
  DrawInfo info = pc.infoBuffer[vi.instID];

  VertexOut vo;
  vo.instID = vi.instID;
  vo.pos = mul(info.transform, float4(pos, 0.0f, 1.0f));
  float2 normalized_uv = pos * 0.5f + 0.5f; // DO NOT CLAMP
  vo.uv = normalized_uv * (info.uvBounds.zw - info.uvBounds.xy) + info.uvBounds.xy;
  vo.uv1 = pos;
  return vo;
}

float median(float r, float g, float b) {
  return max(min(r, g), min(max(r, g), b));
}

#define EMRANGE_MIN -1
#define EMRANGE_MAX 1
#define EMTHRESHOLD 0.2

[shader("fragment")]
float4 main(VertexOut vo) : SV_Target {
  float4 frag_out;
  DrawInfo info = pc.infoBuffer[vo.instID];

  float4 texel = (info.texID == 0) ? float4(info.color, 1.0f)
                                   : sample2DLinear(info.texID, vo.uv);

#ifdef DEBUG_FONT_ATLAS
  frag_out = texel;
#else
  if (info.isMSDF != 0) {
    float signDist = median3(texel.r, texel.g, texel.b);
    float w = fwidth(signDist);
    float opacity = smoothstep(0.5 - w, 0.5 + w, signDist);
    frag_out = float4(float3(1.0), opacity);
  } else {
    frag_out = texel;
  }
#endif

  float2 inBound = step(abs(vo.uv1), float2(1.0));
  frag_out.a *= inBound.x * inBound.y;

  return frag_out;
}
