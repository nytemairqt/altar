#!/bin/bash

set -e 

PLUGIN_NAME="Altar.vst3"
VST3_DIR="$HOME/.vst3"

echo "Installing $PLUGIN_NAME..."

if [ ! -d "$PLUGIN_NAME" ]; then
    echo "Error: $PLUGIN_NAME not found in current directory"
    echo "Please make sure $PLUGIN_NAME is in the same directory as this script"
    exit 1
fi

if [ ! -d "$VST3_DIR" ]; then
    echo "Creating VST3 directory: $VST3_DIR"
    mkdir -p "$VST3_DIR"
fi

if [ -d "$VST3_DIR/$PLUGIN_NAME" ]; then
    echo "Removing existing installation..."
    rm -rf "$VST3_DIR/$PLUGIN_NAME"
fi

echo "Copying $PLUGIN_NAME to $VST3_DIR..."
cp -r "$PLUGIN_NAME" "$VST3_DIR/"

chmod -R 755 "$VST3_DIR/$PLUGIN_NAME"

echo "✓ Installation complete!"
echo "Plugin installed to: $VST3_DIR/$PLUGIN_NAME"
echo ""
echo "Your DAW should now be able to find the Altar VST3 plugin."
echo "You may need to rescan your plugins in your DAW."