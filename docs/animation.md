How animation system work
'''
[glTF .glb / .anim.json]
   │
   ├─► Scene::load_gltf: model.skins -> skeletons (invBindMats, topological parentIndices)
   │                      model.animations -> actions (baked Sim3 track keyframes)
   │                      JOINTS_0, WEIGHTS_0 -> Mesh::SkinningVertex & GPU skinning buffer
   └─► Scene::load_anim_metadata: links puppets, mesh IDs, and action hashes
            │
[ECS Gameplay Loop (demo_anim)]
   │
   ├─► ecs::sys::animation:
   │     1. Action::sample(t, pose)     ─── frame lookup + nlerp/mix
   │     2. forward_kinematics(...)     ─── Non-recursive topological FK + invBindMats
   │     3. Stream skinningMatrices     ─── CPU-to-GPU storage buffer
   │
[Renderer3D & Vulkan Pipeline]
   │
   ├─► Pipe::Skinning (Compute):
   │     - Dispatches skinning.slang with deforms, skinning, rest vertices (iBuf) -> skinned vertices (oBuf)
   │     - vkCmdPipelineBarrier2: COMPUTE_SHADER write ──► VERTEX_SHADER read
   └─► Pipe::Mesh (Graphics):
         - Binds skinnedMeshAddress as vertexBuffer with modelMat in PushConstants
'''
