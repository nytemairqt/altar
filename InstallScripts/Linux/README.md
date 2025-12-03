# MyPlugin VST3 Installer

This installer packages the `Altar.vst3` bundle for Linux. You can install to the user VST3 directory (`~/.vst3`) or the system directory (`/usr/lib/vst3`).

## Install

```bash
chmod +x install.sh
./install.sh --user
# or
sudo ./install.sh --system
```

Options:
- `--force` to overwrite an existing installation
- `--dry-run` to preview actions

## Uninstall

```bash
./uninstall.sh --user
# or
sudo ./uninstall.sh --system
```

## DAW Scanning

After installation, rescan your plugins in your DAW. Some DAWs only scan `/usr/lib/vst3`, so use `--system` installation if needed.
