#!/bin/bash
set -euo pipefail

root=$(cd "$(dirname "$0")/.." && pwd)
build=${KEEK_BUILD_DIR:-$root/build}
config=${XDG_CONFIG_HOME:-$HOME/.config}
shortcut=true
autostart=false

usage() {
    echo "Usage: $0 [--autostart] [--no-shortcut]"
    echo "Installs Keek under the build's install prefix and gives Space to Quick Look in Dolphin."
    echo "  --autostart    also start keekd at login"
    echo "  --no-shortcut  leave Dolphin's shortcuts alone"
}

for argument in "$@"; do
    case $argument in
    --autostart) autostart=true ;;
    --no-shortcut) shortcut=false ;;
    -h | --help)
        usage
        exit 0
        ;;
    *)
        usage >&2
        exit 1
        ;;
    esac
done

if [ ! -x "$build/bin/keekd" ]; then
    echo "No build in $build. Build first: cmake --workflow --preset build" >&2
    exit 1
fi

if $shortcut && ! "$root/scripts/dolphin-shortcut.sh" check; then
    echo "Run this again once Dolphin is closed, or pass --no-shortcut to install without binding Space." >&2
    exit 1
fi

prefix=$(sed -n 's/^CMAKE_INSTALL_PREFIX:PATH=//p' "$build/CMakeCache.txt")
cmake --install "$build" >/dev/null
echo "Installed under $prefix"

busctl --user call org.freedesktop.DBus /org/freedesktop/DBus org.freedesktop.DBus ReloadConfig >/dev/null 2>&1 || true
"$prefix/bin/keek" quit >/dev/null 2>&1 || true

if $shortcut; then
    "$root/scripts/dolphin-shortcut.sh" bind
    echo "Space now opens Quick Look in Dolphin. Select Files and Folders has no shortcut; it is still in the Edit menu."
fi

if $autostart; then
    install -D -m 644 "$prefix/share/keek/keekd.desktop" "$config/autostart/keekd.desktop"
    echo "keekd will start at login."
fi
