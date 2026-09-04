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

echo "Using Blender at: $BLENDER_PATH"
"$BLENDER_PATH" --background "$MODEL_DIR/player.blend" --python "$SCRIPT_DIR/export_player.py"
