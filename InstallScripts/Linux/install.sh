#!/usr/bin/env bash
set -euo pipefail

# copy this install.sh script & uninstall.sh to payload folder:
# /altar/Binaries/Builds/LinuxMakeFile/build

# to package installer:
# sudo apt install makeself
# makeself --bzip2 build Altar-Installer.run "Altar VST3 Installer" ./install.sh

# to test installer:
# chmod +x Altar-Installer.run
# ./Altar-Installer.run

PLUGIN_NAME="Altar"
PLUGIN_BUNDLE="Altar.vst3"
USER_VST3_DIR="$HOME/.vst3"
SYSTEM_VST3_DIR="/usr/lib/vst3"

print_header() {
  echo "==============================================="
  echo " $PLUGIN_NAME VST3 Installer"
  echo "==============================================="
}

usage() {
  echo "Usage: $0 [--system | --user] [--force] [--dry-run]"
  echo "  --system  Install to $SYSTEM_VST3_DIR (requires sudo)"
  echo "  --user    Install to $USER_VST3_DIR (default)"
  echo "  --force   Overwrite existing plugin if present"
  echo "  --dry-run Show actions without modifying the system"
}

TARGET="$USER_VST3_DIR"
FORCE=0
DRYRUN=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --system) TARGET="$SYSTEM_VST3_DIR"; shift ;;
    --user) TARGET="$USER_VST3_DIR"; shift ;;
    --force) FORCE=1; shift ;;
    --dry-run) DRYRUN=1; shift ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1"; usage; exit 1 ;;
  esac
done

print_header

# Resolve script directory (payload lives next to this installer script)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PAYLOAD_DIR="$SCRIPT_DIR"
SOURCE="$PAYLOAD_DIR/$PLUGIN_BUNDLE"

if [[ ! -d "$SOURCE" ]]; then
  echo "Error: Plugin bundle '$SOURCE' not found."
  exit 1
fi

echo "Install target: $TARGET"
echo "Plugin source:  $SOURCE"

# Ensure target directory exists
if [[ "$DRYRUN" -eq 0 ]]; then
  mkdir -p "$TARGET"
fi

DEST="$TARGET/$PLUGIN_BUNDLE"

# Check existing installation
if [[ -e "$DEST" ]]; then
  if [[ "$FORCE" -eq 1 ]]; then
    echo "Existing installation detected. Will overwrite due to --force."
    if [[ "$DRYRUN" -eq 0 ]]; then
      rm -rf "$DEST"
    fi
  else
    echo "A plugin already exists at $DEST."
    echo "Use --force to overwrite, or remove it manually."
    exit 1
  fi
fi

# Copy plugin bundle
echo "Installing $PLUGIN_NAME to $TARGET ..."
if [[ "$DRYRUN" -eq 0 ]]; then
  # Use rsync for robust copy if available, else fallback to cp
  if command -v rsync >/dev/null 2>&1; then
    rsync -a --delete "$SOURCE" "$TARGET/"
  else
    cp -r "$SOURCE" "$TARGET/"
  fi

  # Fix permissions (readable by all, executable bits for binaries)
  find "$DEST" -type d -exec chmod 755 {} \;
  find "$DEST" -type f -exec chmod 644 {} \; || true

  # If binary files need exec bit, uncomment:
  # find "$DEST" -type f -name "*.so" -exec chmod 755 {} \; || true
fi

echo "Installation complete."
echo
echo "Notes:"
echo "- User install path: $USER_VST3_DIR"
echo "- System install path: $SYSTEM_VST3_DIR"
echo "- Some DAWs scan only system paths; use --system if needed."
echo
echo "You may need to rescan plugins in your DAW."