import os
script_dir = os.path.dirname(os.path.abspath(__file__))
with open(os.path.join(script_dir, "export_scene.py")) as f:
    exec(f.read())
