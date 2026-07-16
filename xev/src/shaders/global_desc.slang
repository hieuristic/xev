#language slang 2026

implementing mesh;

struct GlobalHeap {
  SamplerState sampler_linear;
  Texture2D textures[];
}

ParameterBlock<GlobalHeap> global_heap;

float4 sample2DLinear(uint32_t tid, float2 uv) {
  if (tid == 0xFFFFFFFF) {
    return float4(1.0);
  }

  return global_heap.textures[tid].Sample(global_heap.sampler_linear, uv);
}
