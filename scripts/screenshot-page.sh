#!/bin/bash
set -euo pipefail

root=$(cd "$(dirname "$0")/.." && pwd)
bin_dir=${KEEK_BIN_DIR:-$root/build/bin}
fixtures_dir=${KEEK_FIXTURES_DIR:-$root/tests/fixtures}
out_dir=${1:-$root/build/screenshots}

if [ -z "${KEEK_PRIVATE_BUS:-}" ]; then
    home=$(mktemp -d)
    trap 'rm -rf "$home"' EXIT
    mkdir -p "$home/config" "$home/data" "$home/cache" "$home/state"
    env KEEK_PRIVATE_BUS=1 \
        XDG_CONFIG_HOME="$home/config" XDG_DATA_HOME="$home/data" \
        XDG_CACHE_HOME="$home/cache" XDG_STATE_HOME="$home/state" \
        QT_QPA_PLATFORM="${KEEK_PLATFORM:-offscreen}" \
        dbus-run-session -- "$0" "$out_dir"
    exit $?
fi

keek="$bin_dir/keek"
mkdir -p "$out_dir"
rm -f "$out_dir"/*.png "$out_dir/index.html"

"$bin_dir/keekd" 2>"$out_dir/keekd.log" &
daemon=$!
trap 'kill "$daemon" 2>/dev/null || true' EXIT

for _ in $(seq 100); do
    "$keek" state >/dev/null 2>&1 && break
    sleep 0.05
done

field() {
    sed -n -E "s/.*\"$1\":\"?([^\",}]*)\"?.*/\1/p" <<<"$2"
}

page="$out_dir/index.html"
cat >"$page" <<'HTML'
<!doctype html>
<html lang="en">
<meta charset="utf-8">
<title>Keek screenshots</title>
<style>
body { font-family: sans-serif; margin: 24px; background: #808080; color: #fff; }
main { display: grid; grid-template-columns: repeat(auto-fill, minmax(420px, 1fr)); gap: 24px; }
figure { margin: 0; }
img { width: 100%; display: block; }
figcaption { padding: 6px 0; font-size: 14px; }
.error { color: #ffd0d0; }
</style>
<h1>Keek screenshots</h1>
<main>
HTML

for file in "$fixtures_dir"/*; do
    name=$(basename "$file")
    "$keek" show "$file"
    state=""
    for _ in $(seq 100); do
        state=$("$keek" state)
        case "$(field status "$state")" in
        ready | error) break ;;
        esac
        sleep 0.05
    done
    sleep 0.1
    "$keek" screenshot "$out_dir/$name.png"
    renderer=$(field renderer "$state")
    status=$(field status "$state")
    printf '%-24s %-12s %s\n' "$name" "$renderer" "$status"
    printf '<figure><img src="%s.png" alt="%s"><figcaption class="%s">%s: %s, %s</figcaption></figure>\n' \
        "$name" "$name" "$status" "$name" "$renderer" "$status" >>"$page"
done

printf '</main>\n</html>\n' >>"$page"

"$keek" quit
echo "Wrote $page"
