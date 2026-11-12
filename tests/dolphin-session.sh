#!/bin/bash
set -uo pipefail

action="servicemenu_keek.desktop::quicklook"

if [ -z "${KEEK_SESSION_DIR:-}" ]; then
    for tool in kwin_wayland dolphin qdbus-qt6 dbus-run-session; do
        if ! command -v "$tool" >/dev/null; then
            echo "Skipped: $tool is not installed"
            exit 77
        fi
    done

    dir=$(mktemp -d /tmp/keek-XXXXXX)
    trap 'rm -rf "$dir"' EXIT
    mkdir -p "$dir/runtime" "$dir/config" "$dir/data/kio/servicemenus" "$dir/cache" "$dir/state" "$dir/files"
    chmod 700 "$dir/runtime"

    sed "s|^Exec=.*|Exec=$KEEK_BIN_DIR/keek show %F|" "$KEEK_SERVICE_MENU" >"$dir/data/kio/servicemenus/keek.desktop"
    chmod +x "$dir/data/kio/servicemenus/keek.desktop"
    cp "$KEEK_FIXTURES_DIR/image-landscape.png" "$dir/files/a.png"
    cp "$KEEK_FIXTURES_DIR/text-plain.txt" "$dir/files/b.txt"
    cp "$KEEK_FIXTURES_DIR/video-clip.webm" "$dir/files/c.webm"

    env -u WAYLAND_DISPLAY -u DISPLAY -u XDG_ACTIVATION_TOKEN -u DBUS_SESSION_BUS_ADDRESS \
        KEEK_SESSION_DIR="$dir" QT_QPA_PLATFORM=wayland \
        XDG_RUNTIME_DIR="$dir/runtime" XDG_CONFIG_HOME="$dir/config" XDG_DATA_HOME="$dir/data" \
        XDG_CACHE_HOME="$dir/cache" XDG_STATE_HOME="$dir/state" \
        timeout 100 dbus-run-session -- kwin_wayland --virtual --width 1920 --height 1080 --socket keek \
        --no-lockscreen --no-global-shortcuts --no-kactivities --exit-with-session "$0" >"$dir/kwin.log" 2>&1

    cat "$dir/result.log" 2>/dev/null
    if [ -f "$dir/passed" ]; then
        exit 0
    fi
    echo "--- keekd log"
    cat "$dir/keekd.log" 2>/dev/null
    echo "--- kwin log"
    tail -n 40 "$dir/kwin.log" 2>/dev/null
    exit 1
fi

dir=$KEEK_SESSION_DIR
exec >"$dir/result.log" 2>&1
keek="$KEEK_BIN_DIR/keek"
dolphin_pid=""

fail() {
    echo "FAILED: $*"
    echo "state: $("$keek" state 2>&1)"
    [ -n "$dolphin_pid" ] && kill "$dolphin_pid" 2>/dev/null
    "$keek" quit >/dev/null 2>&1
    exit 1
}

wait_for() {
    local tries=$1
    shift
    for _ in $(seq "$tries"); do
        "$@" >/dev/null 2>&1 && return 0
        sleep 0.1
    done
    return 1
}

action_registered() {
    qdbus-qt6 "org.kde.dolphin-$dolphin_pid" /dolphin/Dolphin_1 org.kde.KMainWindow.actions | grep -q "$action"
}

state_has() {
    local state
    state=$("$keek" state) || return 1
    for expected in "$@"; do
        grep -q -F -- "$expected" <<<"$state" || return 1
    done
}

start_dolphin() {
    local label=$1
    shift
    dolphin --new-window "$@" >"$dir/dolphin-$label.log" 2>&1 &
    dolphin_pid=$!
    wait_for 200 action_registered || fail "$label: Dolphin did not register the Quick Look action"
}

trigger_until() {
    local label=$1
    shift
    for _ in $(seq 30); do
        qdbus-qt6 "org.kde.dolphin-$dolphin_pid" /dolphin/Dolphin_1 org.kde.KMainWindow.activateAction "$action" >/dev/null
        if wait_for 10 state_has "$@"; then
            return 0
        fi
        "$keek" hide >/dev/null 2>&1
        sleep 0.3
    done
    fail "$label: the popup never reached the expected state: $*"
}

popup_geometry() {
    local match uuid
    match=$(dbus-send --session --print-reply --dest=org.kde.KWin /WindowsRunner org.kde.krunner1.Match string:Keek) || return 1
    uuid=$(grep -o '"[0-9]*_{[0-9a-f-]*}"' <<<"$match" | head -1 | tr -d '"' | sed 's/^[0-9]*_//')
    [ -n "$uuid" ] || return 1
    qdbus-qt6 org.kde.KWin /KWin org.kde.KWin.getWindowInfo "$uuid" | awk -F': ' '$1 == "x" || $1 == "y" || $1 == "width" || $1 == "height" { printf "%s=%s ", $1, $2 }'
}

popup_centred() {
    local geometry state
    geometry=$(popup_geometry) || return 1
    state=$("$keek" state) || return 1
    awk -v g="$geometry" -v w="$(grep -o '"width":[0-9]*' <<<"$state" | cut -d: -f2)" -v h="$(grep -o '"height":[0-9]*' <<<"$state" | cut -d: -f2)" 'BEGIN {
        split(g, parts, " ")
        for (i in parts) { split(parts[i], kv, "="); value[kv[1]] = kv[2] }
        dx = value["x"] * 2 + value["width"] - 1920
        dy = value["y"] * 2 + value["height"] - 1080
        exit !(value["width"] == w && value["height"] == h && dx >= -1 && dx <= 1 && dy >= -1 && dy <= 1)
    }'
}

close_dolphin() {
    "$keek" hide >/dev/null 2>&1
    kill "$dolphin_pid" 2>/dev/null
    wait "$dolphin_pid" 2>/dev/null
    dolphin_pid=""
}

selection_shortcut() {
    busctl --user get-property "org.kde.dolphin-$dolphin_pid" /dolphin/Dolphin_1/actions/toggle_selection_mode org.qtproject.Qt.QAction shortcut
}

KEEK_AUTOPLAY=0 "$KEEK_BIN_DIR/keekd" 2>"$dir/keekd.log" &
wait_for 100 "$keek" state || fail "keekd did not start"

shortcut="$(dirname "$0")/../scripts/dolphin-shortcut.sh"
rc="$dir/data/kxmlgui5/dolphin/dolphinui.rc"
"$shortcut" bind || fail "shortcut: bind failed on a fresh account"

start_dolphin selection --select "$dir/files/a.png" "$dir/files/b.txt"
wait_for 50 grep -q '<ToolBar' "$rc" || fail "shortcut: Dolphin did not merge its own layout into the stub: $(cat "$rc")"
grep -q 'version="1"' "$rc" && fail "shortcut: the merged file kept the stub's version"
[ "$("$shortcut" status)" = bound ] || fail "shortcut: the merged file lost the Quick Look shortcut"
[ "$(selection_shortcut)" = "(ai) 4 0 0 0 0" ] || fail "shortcut: selection mode still has a shortcut: $(selection_shortcut)"
"$shortcut" check 2>/dev/null && fail "shortcut: check passed while Dolphin was open"
"$shortcut" bind || fail "shortcut: binding again while Dolphin is open should change nothing and succeed"
"$shortcut" unbind 2>/dev/null && fail "shortcut: unbind changed the file while Dolphin was open"
[ "$("$shortcut" status)" = bound ] || fail "shortcut: the refused unbind still changed the file"
echo "ok: on a fresh account Dolphin merges the stub, Space leaves selection mode, and edits wait for Dolphin to close"

trigger_until selection '"visible":true' "\"$dir/files/a.png\"" "\"$dir/files/b.txt\"" '"activationToken":true'
wait_for 50 state_has '"status":"ready"' || fail "selection: the preview did not become ready"
wait_for 50 state_has '"active":true' || fail "selection: the popup did not get keyboard focus"
echo "ok: two selected files reach the popup, with an activation token and keyboard focus"
wait_for 30 popup_centred || fail "selection: the popup is not centred at its own size: $(popup_geometry)"
"$keek" next || fail "selection: could not move to the next file"
wait_for 50 state_has '"status":"ready"' "\"$dir/files/b.txt\"" || fail "next: the text preview did not become ready"
wait_for 30 popup_centred || fail "next: the popup is not centred at its new size: $(popup_geometry)"
"$keek" hide
"$keek" show "$dir/files/c.webm" || fail "video: keek show failed"
wait_for 50 state_has '"status":"ready"' '"renderer":"video"' || fail "video: the video preview did not become ready"
wait_for 30 popup_centred || fail "video: the popup is not centred once the video has loaded: $(popup_geometry)"
echo "ok: the popup is centred at its own size, and stays centred when the next file has another size or loads late"
close_dolphin

"$shortcut" unbind || fail "shortcut: unbind failed once Dolphin was closed"
[ "$("$shortcut" status)" = unbound ] || fail "shortcut: still bound after unbind"
start_dolphin unbound "$dir/files"
[ "$(selection_shortcut)" = "(ai) 4 32 0 0 0" ] || fail "shortcut: Space is not back on selection mode: $(selection_shortcut)"
echo "ok: unbinding gives Space back to selection mode"
close_dolphin

"$keek" quit || fail "keekd did not quit"
touch "$dir/passed"
echo "PASSED"
