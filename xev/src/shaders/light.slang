#language slang 2026
// should be in sync with inc/xev/resouce/light.h

implementing mesh;

enum LightType {
  DIRECTIONAL,
  POINT,
  SPOT,
};

struct Light {
  LightType type;

  float3 position;
  float3 direction;
  float3 color;

  float range;
  float intensity;
  float2 offset;
  uint shadowmap_id;
};
