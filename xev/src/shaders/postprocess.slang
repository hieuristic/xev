#language slang 2026

implementing mesh;

float3 ACESFilm(float3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

float3 tonemap(float3 hdr) {
  // float exposure = 0.0001;
  // float3 ldr = ACESFilm(hdr) * exposure;
  // ldr = pow(ldr, 1.0 / 2.2); // gamma correction
  return hdr * 0.00001;
}
