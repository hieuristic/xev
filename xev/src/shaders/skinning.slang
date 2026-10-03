#language slang 2026

module skinning;

import mesh;

struct Skinning {
  uint4 boneIdx;
  float4 weight;
};

struct PushConst {
  const float4x4* deforms;
  const Skinning* skinning;
  const VertexOut* iBuf;
  VertexOut* oBuf;
  uint32_t offset;
  uint32_t maxThread;
};

[[vk::push_constant]]
uniform PushConstants pc;

[shader("compute")]
[numthreads(256,1,1)]
void main(uint32_t threadID : SV_DispatchThreadID) {
  uint32_t idx = threadID.x;
  if (idx >= maxThread) return;

  // front loading
  VertexOut v = pc.iBuf[idx];
  Skinning skinning = pc.skinning[idx];

  uint4 b = pc.offset + skinning.boneIdx;
  float4 w = skinning.weight;

  // LBS
  float4x4 blendedDeform4 = w.x * pc.deforms[b.x] +
                            w.y * pc.deforms[b.y] +
                            w.z * pc.deforms[b.z] +
                            w.w * pc.deforms[b.w];
  // float3x3 blendedDeform3 = float3x3(blendedDeform4);

  v.position = mul(blendedDeform4, float4(v.position, 1.0f)).xyz;
  v.normal = normalize(mul((float3x3)blendedDeform4, v.normal));
  // v.tangent.xyz = blendedDeform3 * v.tangent.xyz;

  pc.oBuf[idx] = v;
}
