#language slang 2026

module skinning;

// import mesh;
struct Vertex {
  float3 position;
  float3 normal;
  float2 uv;
};

struct Palette {
  uint4 boneIdx;
  float4 weight;
};

struct PushConst {
  column_major float4x4* boneTransforms;
  uint4* boneIndices;
  float4* boneWeights;
  Vertex* iVertBuf;
  Vertex* oVertBuf;
  uint32_t offset;
  uint32_t maxThread;
};

[[vk::push_constant]]
uniform PushConst pc;

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint32_t threadID : SV_DispatchThreadID) {
  uint32_t idx = threadID.x;
  if (idx >= pc.maxThread) return;

  // front loading
  Vertex v = pc.iVertBuf[idx];
  Palette palette = pc.palettes[idx];

  uint4 b = pc.offset + palette.boneIdx;
  float4 w = palette.weight;

  // LBS
  float4x4 deformMat4 = w.x * pc.boneTransforms[b.x] +
                        w.y * pc.boneTransforms[b.y] +
                        w.z * pc.boneTransforms[b.z] +
                        w.w * pc.boneTransforms[b.w];
  // float3x3 deformMat3 = float3x3(deformMat4);

  v.position = mul(deformMat4, float4(v.position, 1.0f)).xyz;
  v.normal = normalize(mul((float3x3)deformMat4, v.normal));
  // v.tangent.xyz = deformMat3 * v.tangent.xyz;

  pc.oVertBuf[idx] = v;
}
