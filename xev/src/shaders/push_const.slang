#language slang 2026
// This is in-sync with pipeline/pipeline_mesh.h

implementing mesh;
__include scene;
__include vertex;

struct PushConstants {
  column_major float4x4 view_proj;
  Scene* scene_ptr;
  Vertex* vert_ptr;
  uint32_t mat_id;
};

[[vk::push_constant]]
uniform PushConstants pc;
