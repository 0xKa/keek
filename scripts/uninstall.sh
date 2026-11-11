#!/bin/bash
set -euo pipefail

root=$(cd "$(dirname "$0")/.." && pwd)
build=${KEEK_BUILD_DIR:-$root/build}
config=${XDG_CONFIG_HOME:-$HOME/.config}

case ${1:-} in
"") ;;
-h | --help)
    echo "Usage: $0"
    echo "Removes everything install.sh added and gives Space back to Dolphin."
    exit 0
    ;;
*)
    echo "Usage: $0" >&2
    exit 1
    ;;
esac

if [ ! -f "$build/install_manifest.txt" ]; then
    echo "No install_manifest.txt in $build, so the installed files are not known." >&2
    exit 1
fi

if [ "$("$root/scripts/dolphin-shortcut.sh" status)" = bound ] && ! "$root/scripts/dolphin-shortcut.sh" check; then
    exit 1
fi

prefix=$(sed -n 's/^CMAKE_INSTALL_PREFIX:PATH=//p' "$build/CMakeCache.txt")
"$prefix/bin/keek" quit >/dev/null 2>&1 || true

"$root/scripts/dolphin-shortcut.sh" unbind
rm -f "$config/autostart/keekd.desktop" "$config/keekrc"
cmake --build "$build" --target uninstall >/dev/null
rmdir --ignore-fail-on-non-empty "$prefix/share/keek" 2>/dev/null || true
echo "Keek is removed and Space is back on Select Files and Folders."
