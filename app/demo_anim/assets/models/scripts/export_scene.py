import bpy
import sys
import os
import math
import json
import struct
import argparse

def hash_string(s):
    """32-bit FNV-1a hash matching StringHash."""
    h = 2166136261
    for b in s.encode('utf-8'):
        h = ((h ^ b) * 16777619) & 0xFFFFFFFF
    return h

def extract_actions(obj):
    """Extracts animation actions from NLA tracks or active action."""
    actions = []
    seen = set()

    if not obj.animation_data:
        return actions

    # 1. NLA tracks (e.g., 'hit', 'walk', 'dance')
    for track in obj.animation_data.nla_tracks:
        for strip in track.strips:
            name = track.name if track.name else (strip.action.name if strip.action else "unnamed")
            if name in seen:
                continue
            seen.add(name)
            actions.append({
                "id": hash_string(name),
                "name": name,
                "track": track.name,
                "frame_start": float(strip.frame_start),
                "frame_end": float(strip.frame_end)
            })

    # 2. Active action if not recorded from NLA
    if obj.animation_data.action:
        act = obj.animation_data.action
        if act.name not in seen:
            seen.add(act.name)
            actions.append({
                "id": hash_string(act.name),
                "name": act.name,
                "track": act.name,
                "frame_start": float(act.frame_range[0]),
                "frame_end": float(act.frame_range[1])
            })

    return actions

def get_child_meshes(armature_obj):
    """Finds all mesh objects parented to or deformed by the armature."""
    child_meshes = []
    for obj in bpy.data.objects:
        if obj.type != 'MESH':
            continue
        if obj.parent == armature_obj:
            child_meshes.append(obj.name)
        else:
            for mod in obj.modifiers:
                if mod.type == 'ARMATURE' and getattr(mod, 'object', None) == armature_obj:
                    child_meshes.append(obj.name)
                    break
    return sorted(list(set(child_meshes)))

def resolve_gltf_mesh_indices(glb_path, mesh_object_names):
    """Resolves mesh object names to their actual indices in the exported glTF."""
    try:
        with open(glb_path, 'rb') as f:
            f.seek(12)
            chunk_len, _ = struct.unpack('<II', f.read(8))
            gltf = json.loads(f.read(chunk_len).decode('utf-8'))

        # Map glTF node name -> glTF mesh index
        node_mesh_map = {}
        for node in gltf.get('nodes', []):
            if 'mesh' in node:
                node_mesh_map[node.get('name')] = node['mesh']

        # Map glTF mesh name -> glTF mesh index
        mesh_name_map = {}
        for idx, mesh in enumerate(gltf.get('meshes', [])):
            if 'name' in mesh:
                mesh_name_map[mesh['name']] = idx

        indices = []
        for name in mesh_object_names:
            if name in node_mesh_map:
                indices.append(node_mesh_map[name])
            elif name in mesh_name_map:
                indices.append(mesh_name_map[name])
        return sorted(list(set(indices)))
    except Exception as e:
        print(f"Warning: Failed to parse glTF mesh indices from {glb_path}: {e}")
        return []

def export_scene(blend_path, collections=None, output_glbs=None, output_jsons=None):
    blend_name = os.path.splitext(os.path.basename(blend_path))[0]
    print(f"\n=== Exporting {blend_path} ===")

    # Collections to export
    if not collections:
        collections = [c.name for c in bpy.data.collections] or ["Master Collection"]
    elif isinstance(collections, str):
        collections = [collections]

    target_cols = [bpy.data.collections[c] for c in collections if c in bpy.data.collections]

    # Select renderable objects in target collections
    for obj in bpy.data.objects:
        obj.select_set(False)

    selected_objects = []
    for col in target_cols:
        for obj in col.objects:
            if not obj.hide_render:
                obj.select_set(True)
                selected_objects.append(obj)

    if not selected_objects:
        selected_objects = [o for o in bpy.data.objects if not o.hide_render]
        for obj in selected_objects:
            obj.select_set(True)

    # Sync camera exposure properties to glTF extras if active cam exists
    scene = bpy.context.scene
    exposure_ev = float(scene.view_settings.exposure)
    cam_obj = bpy.data.objects.get("cam.active")
    if cam_obj and cam_obj.type == 'CAMERA':
        cam_obj.data["exposure_ev"] = exposure_ev
        cam_obj.data["exposure"] = float(math.pow(2.0, exposure_ev))

    # Recalculate mesh normals outward
    for obj in selected_objects:
        if obj.type == 'MESH':
            bpy.context.view_layer.objects.active = obj
            try:
                bpy.ops.object.mode_set(mode='EDIT')
                bpy.ops.mesh.select_all(action='SELECT')
                bpy.ops.mesh.normals_make_consistent(inside=False)
            except Exception:
                pass
            finally:
                if bpy.context.object and bpy.context.object.mode != 'OBJECT':
                    bpy.ops.object.mode_set(mode='OBJECT')

    # 1. Export animated .glb first (export_apply=False is required for armature skinning)
    for glb_path in output_glbs:
        os.makedirs(os.path.dirname(os.path.abspath(glb_path)), exist_ok=True)
        bpy.ops.export_scene.gltf(
            filepath=glb_path,
            export_format='GLB',
            use_selection=True,
            export_animations=True,
            export_animation_mode='NLA_TRACKS',
            export_skins=True,
            export_apply=False,
            export_yup=True,
            export_cameras=True,
            export_lights=True,
            export_extras=True,
            export_attributes=True
        )
        print(f"Exported GLB -> {glb_path} ({os.path.getsize(glb_path)} bytes)")

    # 2. Build clean metadata: ONLY "scene", "puppet", "action"
    # Resolve mesh indices directly from exported glTF
    primary_glb = output_glbs[0] if output_glbs else None
    puppets = []
    actions = []
    seen_action_ids = set()

    for obj in selected_objects:
        is_armature = (obj.type == 'ARMATURE')
        has_anim = obj.animation_data and (obj.animation_data.nla_tracks or obj.animation_data.action)

        if is_armature or has_anim:
            obj_actions = extract_actions(obj)
            child_mesh_names = get_child_meshes(obj) if is_armature else []
            mesh_indices = resolve_gltf_mesh_indices(primary_glb, child_mesh_names) if primary_glb else []

            puppets.append({
                "id": hash_string(obj.name),
                "name": obj.name,
                "meshes": mesh_indices,
                "actions": [a["id"] for a in obj_actions]
            })

            for a in obj_actions:
                if a["id"] not in seen_action_ids:
                    seen_action_ids.add(a["id"])
                    actions.append(a)

    metadata = {
        "scene": blend_name,
        "puppet": puppets,
        "action": actions
    }

    # 3. Export .anim.json metadata
    for json_path in output_jsons:
        os.makedirs(os.path.dirname(os.path.abspath(json_path)), exist_ok=True)
        with open(json_path, 'w', encoding='utf-8') as f:
            json.dump(metadata, f, indent=2)
        print(f"Exported metadata -> {json_path}")

if __name__ == "__main__":
    argv = sys.argv
    custom_args = argv[argv.index("--") + 1:] if "--" in argv else []

    parser = argparse.ArgumentParser(description="Export animated scene and metadata from Blender")
    parser.add_argument("--blend", type=str, default="", help="Path to input .blend file")
    parser.add_argument("--output-glb", nargs="+", default=[], help="Paths for output .glb files")
    parser.add_argument("--output-json", nargs="+", default=[], help="Paths for output .anim.json files")
    parser.add_argument("--collections", nargs="+", default=None, help="Collections to export")
    args = parser.parse_args(custom_args)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    model_dir = os.path.abspath(os.path.join(script_dir, ".."))

    blend_file = args.blend or bpy.data.filepath or os.path.join(model_dir, "scene.blend")
    blend_name = os.path.splitext(os.path.basename(blend_file))[0]

    out_glbs = args.output_glb or [
        os.path.join(model_dir, f"{blend_name}.glb"),
        os.path.join(model_dir, "player_anim.glb")
    ]
    out_jsons = args.output_json or [
        os.path.join(model_dir, f"{blend_name}.anim.json"),
        os.path.join(model_dir, "player_anim.anim.json")
    ]

    collections = args.collections or ["player", "map"]

    export_scene(
        blend_path=blend_file,
        collections=collections,
        output_glbs=out_glbs,
        output_jsons=out_jsons
    )
