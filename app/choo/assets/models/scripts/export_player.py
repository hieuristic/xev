import bpy
import sys
import os
import math
import numpy as np

def setup_gltf_node_group():
    """Creates or retrieves the 'glTF Material Output' node group with Occlusion socket."""
    gltf_group = bpy.data.node_groups.get("glTF Material Output")
    if not gltf_group:
        gltf_group = bpy.data.node_groups.new("glTF Material Output", "ShaderNodeTree")
        if hasattr(gltf_group, "interface"):
            gltf_group.interface.new_socket(name="Occlusion", in_out='INPUT', socket_type='NodeSocketFloat')
        else:
            gltf_group.inputs.new("NodeSocketFloat", "Occlusion")
    return gltf_group

def export_collections(blend_path, collections, output_paths, ao_res=512, ao_samples=32):
    if isinstance(collections, str):
        collection_names = [collections]
    else:
        collection_names = list(collections)

    print(f"=== Processing {blend_path} for collections: {collection_names} ===")

    target_cols = []
    for col_name in collection_names:
        col = bpy.data.collections.get(col_name)
        if not col:
            print(f"Error: Collection '{col_name}' not found in {blend_path}!")
            print(f"Available collections: {[c.name for c in bpy.data.collections]}")
            sys.exit(1)
        target_cols.append(col)

    # 1. Hide unwanted collections from Cycles render bake
    for col in bpy.data.collections:
        if col.name not in collection_names:
            col.hide_render = True
        else:
            col.hide_render = False

    # 2. If shirt exists and body exists, hide duplicate body from render to avoid z-fighting/bake overlap
    body_obj = bpy.data.objects.get("hieu.body")
    shirt_obj = bpy.data.objects.get("hieu.body.shirt")
    if body_obj and shirt_obj:
        print("Hiding overlapping 'hieu.body' in favor of 'hieu.body.shirt' for clean bake...")
        body_obj.hide_render = True
        body_obj.hide_set(True)

    # Read exposure and camera properties dynamically from the .blend file (scene & camera)
    scene = bpy.context.scene
    blender_exposure_ev = float(scene.view_settings.exposure)
    exposure_multiplier = float(math.pow(2.0, blender_exposure_ev))

    cam_obj = bpy.data.objects.get("cam.active")
    if cam_obj and cam_obj.type == 'CAMERA':
        cam_data = cam_obj.data
        cam_data["exposure_ev"] = blender_exposure_ev
        cam_data["exposure"] = exposure_multiplier
        if hasattr(cam_data, "dof") and cam_data.dof:
            cam_data["aperture"] = float(cam_data.dof.aperture_fstop)
        
        # Also copy any custom camera properties defined in the blend file
        for k, v in cam_obj.items():
            if isinstance(v, (int, float, str)):
                cam_data[k] = v

        print(f"Dynamically read from .blend -> EV: {blender_exposure_ev}, Linear Exposure: {exposure_multiplier}, f-stop: {getattr(cam_data.dof, 'aperture_fstop', 'N/A')}")

    # 3. Recalculate normals outward on player meshes (fixes inside-out normals on glasses, shirt, etc.)
    player_col = bpy.data.collections.get("player")
    player_objects = [o for o in player_col.objects if o.type == 'MESH'] if player_col else []
    for obj in player_objects:
        bpy.context.view_layer.objects.active = obj
        try:
            bpy.ops.object.mode_set(mode='EDIT')
            bpy.ops.mesh.select_all(action='SELECT')
            bpy.ops.mesh.normals_make_consistent(inside=False)
        except Exception as e:
            print(f"Warning fixing normals on {obj.name}: {e}")
        finally:
            bpy.ops.object.mode_set(mode='OBJECT')

    # 4. Standardize materials: convert Hair BSDF to Principled BSDF
    for mat in bpy.data.materials:
        if mat.node_tree:
            hair_node = next((n for n in mat.node_tree.nodes if n.type == 'BSDF_HAIR'), None)
            if hair_node:
                color = hair_node.inputs['Color'].default_value[:]
                output_node = next((n for n in mat.node_tree.nodes if n.type == 'OUTPUT_MATERIAL'), None)
                principled = mat.node_tree.nodes.new(type='ShaderNodeBsdfPrincipled')
                principled.inputs['Base Color'].default_value = color
                principled.inputs['Roughness'].default_value = 0.8
                principled.inputs['Metallic'].default_value = 0.0
                if output_node:
                    mat.node_tree.links.new(principled.outputs['BSDF'], output_node.inputs['Surface'])
                print(f"Converted Hair BSDF on material '{mat.name}' to Principled BSDF (color={color})")

    # 5. Setup Cycles engine for raytraced AO bake
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.bake_type = 'AO'
    scene.cycles.samples = ao_samples
    scene.cycles.device = 'CPU'

    # 6. Create bake textures and perform raytraced AO bake for each player mesh
    baked_materials = set()
    for obj in player_objects:
        if obj.hide_render:
            continue

        bpy.ops.object.select_all(action='DESELECT')
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj

        for slot in obj.material_slots:
            mat = slot.material
            if not mat or not mat.node_tree or mat.name in baked_materials:
                continue

            nodes = mat.node_tree.nodes
            bake_img_name = f"{mat.name}_AO_Raw"
            bake_img = bpy.data.images.get(bake_img_name) or bpy.data.images.new(bake_img_name, ao_res, ao_res)
            
            # Temporary texture node to receive bake
            tex_bake = nodes.get("TMP_AO_BAKE") or nodes.new("ShaderNodeTexImage")
            tex_bake.name = "TMP_AO_BAKE"
            tex_bake.image = bake_img
            nodes.active = tex_bake

        print(f"Baking raytraced AO for mesh '{obj.name}'...")
        try:
            bpy.ops.object.bake(type='AO')
            for slot in obj.material_slots:
                if slot.material:
                    baked_materials.add(slot.material.name)
        except Exception as e:
            print(f"Bake failed on {obj.name}: {e}")

    # 7. Pack baked AO into glTF standard ORM textures (R=Occlusion, G=Roughness, B=Metallic)
    gltf_group = setup_gltf_node_group()

    for mat_name in baked_materials:
        mat = bpy.data.materials.get(mat_name)
        if not mat or not mat.node_tree:
            continue
        
        nodes = mat.node_tree.nodes
        principled = next((n for n in nodes if n.type == 'BSDF_PRINCIPLED'), None)
        bake_node = nodes.get("TMP_AO_BAKE")

        if principled and bake_node and bake_node.image:
            raw_ao_img = bake_node.image
            w, h = raw_ao_img.size[0], raw_ao_img.size[1]
            if w == 0 or h == 0:
                continue

            # Extract AO pixels
            ao_raw = np.empty(w * h * 4, dtype=np.float32)
            raw_ao_img.pixels.foreach_get(ao_raw)
            ao_channel = ao_raw[0::4]  # Grayscale AO

            # Get Roughness & Metallic factors
            rough_val = principled.inputs['Roughness'].default_value if 'Roughness' in principled.inputs else 0.5
            metal_val = principled.inputs['Metallic'].default_value if 'Metallic' in principled.inputs else 0.0

            # Pack ORM array (R=AO, G=Roughness, B=Metallic, A=1.0)
            orm_arr = np.empty((h, w, 4), dtype=np.float32)
            orm_arr[:, :, 0] = ao_channel.reshape((h, w))
            orm_arr[:, :, 1] = rough_val
            orm_arr[:, :, 2] = metal_val
            orm_arr[:, :, 3] = 1.0

            orm_img_name = f"{mat.name}_ORM"
            orm_img = bpy.data.images.get(orm_img_name) or bpy.data.images.new(orm_img_name, w, h)
            orm_img.pixels.foreach_set(orm_arr.ravel())
            orm_img.update()

            # Remove temporary bake node
            nodes.remove(bake_node)

            # Create final ORM texture node
            orm_node = nodes.new("ShaderNodeTexImage")
            orm_node.name = f"{mat.name}_ORM_Node"
            orm_node.image = orm_img

            # Separate Color node for glTF export connections
            sep_node = nodes.new("ShaderNodeSeparateColor")
            mat.node_tree.links.new(orm_node.outputs['Color'], sep_node.inputs['Color'])

            # Hook Roughness & Metallic into Principled BSDF
            mat.node_tree.links.new(sep_node.outputs['Green'], principled.inputs['Roughness'])
            mat.node_tree.links.new(sep_node.outputs['Blue'], principled.inputs['Metallic'])

            # Hook Occlusion into glTF Material Output
            gltf_node = nodes.get("glTF Material Output") or nodes.new("ShaderNodeGroup")
            gltf_node.name = "glTF Material Output"
            gltf_node.node_tree = gltf_group
            mat.node_tree.links.new(sep_node.outputs['Red'], gltf_node.inputs['Occlusion'])

            print(f"Packed ORM texture for material '{mat.name}' (Roughness={rough_val:.2f}, Metallic={metal_val:.2f})")

    # 8. Select only active objects in target collections for export
    for obj in bpy.data.objects:
        obj.select_set(False)

    for col in target_cols:
        for obj in col.objects:
            if not obj.hide_render:
                obj.select_set(True)

    # 9. Export to GLB
    for out_path in output_paths:
        os.makedirs(os.path.dirname(out_path), exist_ok=True)
        bpy.ops.export_scene.gltf(
            filepath=out_path,
            export_format='GLB',
            use_selection=True,
            export_apply=True,
            export_yup=True,
            export_cameras=True,
            export_lights=True,
            export_extras=True,
            export_attributes=True
        )
        print(f"Successfully exported GLB to: {out_path}")

def export_collection(blend_path, collection_name, output_paths, ao_res=512, ao_samples=32):
    export_collections(blend_path, [collection_name], output_paths, ao_res, ao_samples)

if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    model_dir = os.path.abspath(os.path.join(script_dir, ".."))
    root_dir = os.path.abspath(os.path.join(script_dir, "../../../../.."))

    blend_file = os.path.join(model_dir, "player.blend")
    collections = ["player", "map"]

    output_glb_local = os.path.join(model_dir, "player.glb")
    output_glb_assets = os.path.join(root_dir, "assets", "player.glb")

    export_collections(blend_file, collections, [output_glb_local, output_glb_assets])

