import bpy
import sys
import os
import argparse

def configure_collections(layer_col, target_names):
    """Ensure target collections are enabled and non-target / player.* collections are excluded and hidden."""
    name = layer_col.name
    if name.startswith("player") or (name != "Scene Collection" and name not in target_names):
        layer_col.exclude = True
        layer_col.hide_viewport = True
    elif name in target_names:
        layer_col.exclude = False
        layer_col.hide_viewport = False
    for child in layer_col.children:
        configure_collections(child, target_names)

def export_anim_glb(blend_path, collections, output_path):
    print(f"=== Exporting animation GLB from {blend_path} ===")
    print(f"Target collections: {collections}")
    print(f"Output GLB path: {output_path}")

    # Explicitly hide all non-target collections, especially player.*
    for col in bpy.data.collections:
        if col.name.startswith("player") or col.name not in collections:
            col.hide_render = True
            col.hide_viewport = True
        else:
            col.hide_render = False
            col.hide_viewport = False

    # Ensure target collections exist
    for col_name in collections:
        col = bpy.data.collections.get(col_name)
        if not col:
            print(f"Warning: Collection '{col_name}' not found in {blend_path}!")

    # Configure collections in the active view layer
    vl = bpy.context.view_layer
    configure_collections(vl.layer_collection, collections)

    # Deselect all objects first, and ensure player.* objects are hidden
    for obj in bpy.data.objects:
        obj.select_set(False)
        if obj.name.startswith("player") or obj.name.startswith("hieu.") or obj.name.startswith("ngok."):
            obj.hide_render = True
            obj.hide_viewport = True

    # Select all objects in the target collections that are renderable
    selected_count = 0
    for col_name in collections:
        col = bpy.data.collections.get(col_name)
        if not col:
            continue
        for obj in col.objects:
            if not obj.hide_render:
                obj.select_set(True)
                selected_count += 1
                print(f"  Selected for animation: {obj.name} (type: {obj.type})")

    if selected_count == 0:
        print("Error: No objects selected for export!")
        sys.exit(1)

    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)

    # Export to GLB with animation and skinning
    # NOTE: export_apply MUST be False to preserve armature modifiers and skinning data.
    bpy.ops.export_scene.gltf(
        filepath=output_path,
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

    print(f"Successfully exported animated GLB to: {output_path}")
    print(f"File size: {os.path.getsize(output_path)} bytes")

if __name__ == "__main__":
    # In Blender background mode, custom args come after '--'
    argv = sys.argv
    if "--" in argv:
        custom_args = argv[argv.index("--") + 1:]
    else:
        custom_args = []

    parser = argparse.ArgumentParser(description="Export animated model to GLB from Blender")
    parser.add_argument("--blend", type=str, default="", help="Path to .blend file")
    parser.add_argument("--output", type=str, default="", help="Path to output .glb file")
    parser.add_argument("--collections", nargs="+", default=["animation", "map"], help="Collections to export")
    args = parser.parse_args(custom_args)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    demo_dir = os.path.abspath(os.path.join(script_dir, ".."))

    blend_file = args.blend
    if not blend_file:
        blend_file = os.path.join(demo_dir, "assets", "models", "player.blend")

    out_file = args.output
    if not out_file:
        out_file = os.path.join(demo_dir, "assets", "models", "player_anim.glb")

    export_anim_glb(blend_file, args.collections, out_file)
