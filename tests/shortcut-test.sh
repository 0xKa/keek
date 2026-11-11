#!/bin/bash
set -uo pipefail

helper=$(dirname "$0")/../scripts/dolphin-shortcut.sh
sample=$(dirname "$0")/shortcut/dolphinui.rc
dir=$(mktemp -d /tmp/keek-XXXXXX)
trap 'rm -rf "$dir"' EXIT
rc="$dir/kxmlgui5/dolphin/dolphinui.rc"
quicklook='  <Action name="servicemenu_keek.desktop::quicklook" shortcut="Space"/>'
unbound='  <Action name="toggle_selection_mode" shortcut="; "/>'
failures=0

fail() {
    echo "FAILED: $*"
    failures=$((failures + 1))
}

check() {
    local label=$1
    shift
    "$@" || fail "$label"
}

properties() {
    sed -n '/<ActionProperties/,/<\/ActionProperties>/p' "$rc"
}

start_from() {
    rm -rf "$dir/kxmlgui5"
    mkdir -p "$(dirname "$rc")"
    cp "$sample" "$rc"
    [ $# -eq 0 ] || sed -i "$@" "$rc"
    cp "$rc" "$dir/original"
}

rm -rf "$dir/kxmlgui5"
check "missing: status before" [ "$("$helper" status "$rc")" = unbound ]
check "missing: bind" "$helper" bind "$rc"
check "missing: a stub with an old version is written" grep -q '<gui name="dolphin" version="1">' "$rc"
check "missing: status after bind" [ "$("$helper" status "$rc")" = bound ]
cp "$rc" "$dir/stub"
check "missing: bind again" "$helper" bind "$rc"
check "missing: binding twice changes nothing" cmp -s "$rc" "$dir/stub"
check "missing: unbind" "$helper" unbind "$rc"
check "missing: unbind removes the stub" [ ! -e "$rc" ]
check "missing: unbind leaves no backup" [ ! -e "$rc.before-keek" ]
echo "ok: a missing file gets a stub that unbind removes"

start_from
check "sample: bind" "$helper" bind "$rc"
check "sample: Quick Look gets Space" grep -qxF "$quicklook" <(properties)
check "sample: selection mode loses Space" grep -qxF "$unbound" <(properties)
check "sample: menu and toolbar entries are untouched" [ "$(grep -c '^ *<Action name="toggle_selection_mode"/>$' "$rc")" -eq 2 ]
check "sample: other shortcuts are kept" grep -qF 'shortcut="Ctrl+Shift+F9"' <(properties)
check "sample: the original is backed up" cmp -s "$rc.before-keek" "$dir/original"
check "sample: status" [ "$("$helper" status "$rc")" = bound ]
cp "$rc" "$dir/bound"
check "sample: bind again" "$helper" bind "$rc"
check "sample: binding twice changes nothing" cmp -s "$rc" "$dir/bound"
check "sample: unbind" "$helper" unbind "$rc"
check "sample: unbind restores the file exactly" cmp -s "$rc" "$dir/original"
check "sample: unbind removes the backup" [ ! -e "$rc.before-keek" ]
check "sample: status after unbind" [ "$("$helper" status "$rc")" = unbound ]
echo "ok: binding and unbinding leave the rest of the file as it was"

start_from "s|^ </ActionProperties>|$quicklook\n$unbound\n&|"
check "reordered: bind" "$helper" bind "$rc"
check "reordered: lines in Dolphin's order count as bound" cmp -s "$rc" "$dir/original"
check "reordered: no backup for a change that was not made" [ ! -e "$rc.before-keek" ]
echo "ok: a file Dolphin rewrote in its own order is already bound"

start_from 's|^  <Action name="show_preview".*|  <Action name="toggle_selection_mode" shortcut="Space; Ctrl+Shift+S"/>|'
check "custom: bind" "$helper" bind "$rc"
check "custom: only Space is taken off selection mode" grep -qF '<Action name="toggle_selection_mode" shortcut="; Ctrl+Shift+S"/>' <(properties)
check "custom: no second selection mode line" [ "$(properties | grep -c toggle_selection_mode)" -eq 1 ]
check "custom: unbind" "$helper" unbind "$rc"
check "custom: unbind keeps the user's own shortcut" grep -qF 'shortcut="; Ctrl+Shift+S"' <(properties)
echo "ok: a custom selection mode shortcut keeps its other keys"

start_from '/<ActionProperties/,/<\/ActionProperties>/c\ <ActionProperties scheme="Default"/>'
check "empty: bind" "$helper" bind "$rc"
check "empty: Quick Look gets Space" grep -qxF "$quicklook" <(properties)
check "empty: status" [ "$("$helper" status "$rc")" = bound ]
echo "ok: an empty ActionProperties element is filled in"

start_from '/<ActionProperties/,/<\/ActionProperties>/d'
check "none: bind" "$helper" bind "$rc"
check "none: Quick Look gets Space" grep -qxF "$quicklook" <(properties)
check "none: the element closes before </gui>" [ "$(tail -2 "$rc" | head -1)" = " </ActionProperties>" ]
echo "ok: a file without ActionProperties gets the element"

start_from 's|^ <ActionProperties scheme="Default">| <State name="x"/><ActionProperties scheme="Default">|'
"$helper" bind "$rc" 2>/dev/null
check "odd: bind refuses a file it does not understand" [ $? -eq 2 ]
check "odd: the file is unchanged" cmp -s "$rc" "$dir/original"
check "odd: no backup is made" [ ! -e "$rc.before-keek" ]
echo "ok: a file in an unexpected layout is left alone"

[ "$failures" -eq 0 ]
