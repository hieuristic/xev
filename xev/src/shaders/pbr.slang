module pbr;

import constants;

// Trowbridge-Reitz distribution (aka GGX)
public float G_TR(float nh, float alpha) {
  float a = alpha * nh;
  float b = alpha / (a * a - nh * nh + 1);
  return b * b / PI;
}

public float G_Berry(float nh, float alpha) {
  float a = alpha * alpha - 1;
  float b = 1 + nh * nh * a;
  return a / (PI * log(alpha * alpha) * b);
}

// generalization of G_TR (gamma = 2) and G_Berry (gamma = 1)
// this function is expensive.
public float G_GTR(float nh, float gamma, float alpha) {
  float a = alpha * alpha;
  float b = a - 1.0;
  float c = 1 - gamma;
  float d = (1.0 - pow(a, -c)) * pow(1 + nh*nh*b, gamma);
  return PI_INV * c * b / d;
}

public float V_Smith(float nv, float nl, float alpha) {
  float a = alpha * alpha;
  float v_ggx = nl * sqrt(nv * nv * (1 - a) + a);
  float l_ggx = nv * sqrt(nl * nl * (1 - a) + a);
  return 0.5 / max(v_ggx + l_ggx, 0.00001);
}

public float F_Schlick(float u, float f0, float f90) {
  return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

public float3 F_Schlick(float u, float3 f0, float f90) {
  return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

public float fd_Lambert() {
  return PI_INV;
}

public float fd_Burley(float nv, float nl, float lh, float alpha) {
  float f90 = 0.5 + 2.0 * alpha * lh * lh;
  return F_Schlick(nl, 1.0, f90) * F_Schlick(nv, 1.0, f90) * PI_INV;
}

public float3 brdf(float3 diffuse, float roughness, float metallic, float3 f0,
                  float3 v, float3 n, float3 l, float3 h) {
  float nl = dot(n, l);
  float nh = dot(n, h);
  float nv = dot(n, v);
  float hv = dot(h, v);
  float alpha = roughness * roughness;

  float3 fr = (G_TR(nh, alpha) * V_Smith(nv, nl, alpha)) * F_Schlick(hv, f0, 1.0);
  float fd = fd_Lambert();
  return diffuse * (fd + fr);
}
