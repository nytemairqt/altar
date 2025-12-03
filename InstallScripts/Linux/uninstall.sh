#!/usr/bin/env bash
set -euo pipefail

# Requires makeself 
# sudo apt install makeself
# copy this script to the "build" payload folder /altar/Binaries/Builds/LinuxMakeFile/build

PLUGIN_BUNDLE="Altar.vst3"
USER_VST3_DIR="$HOME/.vst3"
SYSTEM_VST3_DIR="/usr/lib/vst3"

usage() {
  echo "Usage: $0 [--system | --user] [--dry-run]"
  echo "  --system  Uninstall from $SYSTEM_VST3_DIR"
  echo "  --user    Uninstall from $USER_VST3_DIR (default)"
  echo "  --dry-run Show actions without modifying the system"
}

TARGET="$USER_VST3_DIR"
DRYRUN=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --system) TARGET="$SYSTEM_VST3_DIR"; shift ;;
    --user) TARGET="$USER_VST3_DIR"; shift ;;
    --dry-run) DRYRUN=1; shift ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1"; usage; exit 1 ;;
  esac
done

DEST="$TARGET/$PLUGIN_BUNDLE"

echo "Uninstalling from: $DEST"

if [[ ! -e "$DEST" ]]; then
  echo "No installation found at $DEST"
  exit 0
fi

if [[ "$DRYRUN" -eq 1 ]]; then
  echo "Would remove: $DEST"
else
  rm -rf "$DEST"
  echo "Removed: $DEST"
fi

echo "Uninstall complete."