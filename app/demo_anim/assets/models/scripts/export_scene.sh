#!/usr/bin/env bash
set -e

# Find Blender executable
BLENDER_PATH=""

if command -v blender &> /dev/null; then
    BLENDER_PATH="blender"
elif [ -f "/Users/$USER/Library/Application Support/Steam/steamapps/common/Blender/Blender.app/Contents/MacOS/Blender" ]; then
    BLENDER_PATH="/Users/$USER/Library/Application Support/Steam/steamapps/common/Blender/Blender.app/Contents/MacOS/Blender"
elif [ -f "/Applications/Blender.app/Contents/MacOS/Blender" ]; then
    BLENDER_PATH="/Applications/Blender.app/Contents/MacOS/Blender"
fi

if [ -z "$BLENDER_PATH" ]; then
    echo "Error: Blender executable not found."
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MODEL_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Default input blend file is scene.blend in parent directory, or first positional argument
BLEND_FILE="$MODEL_DIR/scene.blend"
if [ -n "$1" ] && [[ "$1" != --* ]]; then
    BLEND_FILE="$1"
    shift
fi

echo "Using Blender at: $BLENDER_PATH"
echo "Processing blend file: $BLEND_FILE"

"$BLENDER_PATH" --background "$BLEND_FILE" --python "$SCRIPT_DIR/export_scene.py" -- "$@"
