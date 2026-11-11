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

    env -u WAYLAND_DISPLAY -u DISPLAY -u XDG_ACTIVATION_TOKEN -u DBUS_SESSION_BUS_ADDRESS \
        KEEK_SESSION_DIR="$dir" QT_QPA_PLATFORM=wayland \
        XDG_RUNTIME_DIR="$dir/runtime" XDG_CONFIG_HOME="$dir/config" XDG_DATA_HOME="$dir/data" \
        XDG_CACHE_HOME="$dir/cache" XDG_STATE_HOME="$dir/state" \
        timeout 100 dbus-run-session -- kwin_wayland --virtual --socket keek \
        --no-lockscreen --no-global-shortcuts --no-kactivities --exit-with-session "$0" >"$dir/kwin.log" 2>&1

    cat "$dir/result.log" 2>/dev/null
    if [ -f "$dir/passed" ]; then
        exit 0
    fi
    echo "--- keekd log"
    cat "$dir/keekd.log" 2>/dev/null
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

close_dolphin() {
    "$keek" hide >/dev/null 2>&1
    kill "$dolphin_pid" 2>/dev/null
    wait "$dolphin_pid" 2>/dev/null
    dolphin_pid=""
}

"$KEEK_BIN_DIR/keekd" 2>"$dir/keekd.log" &
wait_for 100 "$keek" state || fail "keekd did not start"

start_dolphin selection --select "$dir/files/a.png" "$dir/files/b.txt"
trigger_until selection '"visible":true' "\"$dir/files/a.png\"" "\"$dir/files/b.txt\"" '"activationToken":true'
wait_for 50 state_has '"status":"ready"' || fail "selection: the preview did not become ready"
wait_for 50 state_has '"active":true' || fail "selection: the popup did not get keyboard focus"
echo "ok: two selected files reach the popup, with an activation token and keyboard focus"
close_dolphin

"$keek" quit || fail "keekd did not quit"
touch "$dir/passed"
echo "PASSED"
