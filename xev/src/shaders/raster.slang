#language slang 2026

module raster;
import global_desc;

struct PushConstants {
  column_major float4x4 transform;
  float2 topleft_uv;
  float2 botright_uv;
  uint32_t tex_id;
  uint32_t is_msdf;
};

[[vk::push_constant]]
uniform PushConstants pc;

struct Vertex {
  uint vid        : SV_VertexID;
  out float4 pos  : SV_Position;
  out float2 uv   : TEXCOORD0;
};

[shader("vertex")]
void main(Vertex vert) {
  float x = (float((vert.vid << 1) & 2) * 2.0f) - 1.0f;
  float y = (float(vert.vid & 2) * 2.0f) - 1.0f;
  vert.pos = transform * float4(x, y, 0.0f, 1.0f);
  vert.uv = position.xy * 0.5f + 0.5f;
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

#define FONT_SIZE_PIXELS 32

[shader("fragment")]
float4 main(Vertex vert) {
  float4 frag_out;
  float4 texel = sample2DLinear(pc.texid, vert.uv);
  if (is_msdf) {
    float sd = median3(texel.r, texel.g, texel.b);
    float pxd = FONT_SIZE_PIXELS * (sd - 0.5);
  } else {
  }
  return frag_out;
}
