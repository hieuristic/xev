#language slang 2026

module raster;
import global_desc;

struct DrawInfo {
  float4x4 transform;
  float4 uvBounds; // left top right bottom
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
  out float4 pos  : SV_Position;
  out float2 uv   : TEXCOORD0;
};

static const float2 billboardCorners[3] = {
    float2(-1.0, -1.0),
    float2( 3.0, -1.0),
    float2(-1.0,  3.0)
};

[shader("vertex")]
void main(VertexIn vi) {
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
  vo.instID = vi.instID
  vert.pos = info.transform * float4(pos, 0.0f, 1.0f);
  vert.uv = (pos * 0.5f + 0.5f) * (info.uvBounds.zw - info.uvBounds.xy) + info.uvBounds.xy;;
}

float median(float r, float g, float b) {
  return max(min(r, g), min(max(r, g), b));
}

#define EMRANGE_MIN -1
#define EMRANGE_MAX 1
#define EMTHRESHOLD 0.2

[shader("fragment")]
float4 main(VertexOut vo) {
  float4 frag_out;
  DrawInfo info = pc.infoBuffer[vo.instID];

  float4 texel = sample2DLinear(pc.texid, vert.uv);

  if (is_msdf) {
    float distMix = median3(texel.r, texel.g, texel.b);
    float distEM = mix(EMRANGE_MIN, EMRANGE_MAX, distMix);
    float inverse_width = u_screen_px_scale * u_antialias_per_em;
    float opacity = clamp((u_threshold_em - distance_em) * inverse_width + 0.5, 0.0, 1.0);
    frag_out = float4(float3(opacity), 1.0);
  } else {
    frag_out = texel;
  }

  return frag_out;
}
