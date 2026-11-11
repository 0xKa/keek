#!/bin/bash
set -euo pipefail

usage() {
    echo "Usage: $0 bind|unbind|status|check [dolphinui.rc]"
    echo "Gives Space to Keek's Quick Look action in Dolphin, takes it back, or reports which."
    echo "check fails while a Dolphin window is open, because the file must then not be edited."
}

[ $# -ge 1 ] && [ $# -le 2 ] || {
    usage >&2
    exit 1
}
command=$1
file=${2:-${XDG_DATA_HOME:-$HOME/.local/share}/kxmlgui5/dolphin/dolphinui.rc}
backup="$file.before-keek"

edit() {
    awk -v mode="$1" '
    function fail(message) {
        print "Cannot edit " FILENAME ": " message > "/dev/stderr"
        failed = 1
        exit 2
    }
    function ours(indent) {
        if (!selection) {
            print indent "<Action name=\"toggle_selection_mode\" shortcut=\"; \"/>"
        }
        if (!quicklook) {
            print indent "<Action name=\"servicemenu_keek.desktop::quicklook\" shortcut=\"Space\"/>"
        }
    }
    function withoutSpace(line,    value, parts, count, i, result) {
        if (!match(line, /shortcut="[^"]*"/)) {
            sub(/name="toggle_selection_mode"/, "& shortcut=\"; \"", line)
            return line
        }
        value = substr(line, RSTART + 10, RLENGTH - 11)
        count = split(value, parts, "; ")
        result = ""
        for (i = 1; i <= count; ++i) {
            result = result (i > 1 ? "; " : "") (parts[i] == "Space" ? "" : parts[i])
        }
        if (result == "") {
            result = "; "
        }
        return substr(line, 1, RSTART - 1) "shortcut=\"" result "\"" substr(line, RSTART + RLENGTH)
    }
    /<ActionProperties/ {
        if (!match($0, /^[ \t]*<ActionProperties[ \t>\/]/)) {
            fail("the ActionProperties element does not start its own line")
        }
        if (seen) {
            fail("there is more than one ActionProperties element")
        }
        seen = 1
        indent = substr($0, 1, RLENGTH - 18)
        if ($0 ~ /\/>[ \t]*$/) {
            if (mode == "bind") {
                line = $0
                sub(/[ \t]*\/>[ \t]*$/, ">", line)
                print line
                ours(indent " ")
                print indent "</ActionProperties>"
            } else {
                print
            }
            next
        }
        inside = 1
        print
        next
    }
    inside && /<\/ActionProperties>/ {
        if ($0 !~ /^[ \t]*<\/ActionProperties>[ \t]*$/) {
            fail("the end of the ActionProperties element does not stand on its own line")
        }
        if (mode == "bind") {
            ours(indent " ")
        }
        inside = 0
        print
        next
    }
    inside && /<Action / {
        if (gsub(/<Action /, "&") > 1) {
            fail("two actions share a line")
        }
    }
    inside && /name="servicemenu_keek\.desktop::quicklook"/ {
        if (mode == "status") {
            bound = /shortcut="Space"/
        } else if (mode == "bind" && /^[ \t]*<Action name="servicemenu_keek\.desktop::quicklook" shortcut="Space"\/>[ \t]*$/) {
            quicklook = 1
            print
        }
        next
    }
    inside && /name="toggle_selection_mode"/ {
        if (mode == "bind") {
            selection = 1
            print withoutSpace($0)
        } else if (mode == "status" || $0 !~ /^[ \t]*<Action name="toggle_selection_mode" shortcut="; "\/>[ \t]*$/) {
            print
        }
        next
    }
    /<\/gui>/ && !seen && mode == "bind" {
        seen = 1
        print " <ActionProperties scheme=\"Default\">"
        ours("  ")
        print " </ActionProperties>"
    }
    { print }
    END {
        if (failed) {
            exit 2
        }
        if (inside) {
            fail("the ActionProperties element is not closed")
        }
        if (mode == "bind" && !seen) {
            fail("there is no </gui> to add the shortcut before")
        }
        if (mode == "status") {
            exit bound ? 0 : 3
        }
    }
    ' "$file"
}

dolphin_open() {
    local name
    for name in $(busctl --user list --no-legend 2>/dev/null | awk '$1 ~ /^org\.kde\.dolphin-[0-9]+$/ { print $1 }'); do
        busctl --user tree --list "$name" 2>/dev/null | grep -q '^/dolphin/Dolphin_' && return 0
    done
    return 1
}

refuse_while_dolphin_is_open() {
    if dolphin_open; then
        echo "Dolphin is running. Close every Dolphin window first: a running Dolphin keeps its shortcuts in memory and can overwrite the change." >&2
        exit 4
    fi
}

write_edited() {
    local temporary
    temporary=$(mktemp "$file.XXXXXX")
    if ! edit "$1" >"$temporary"; then
        rm -f "$temporary"
        exit 2
    fi
    if cmp -s "$temporary" "$file"; then
        rm -f "$temporary"
        return
    fi
    if dolphin_open; then
        rm -f "$temporary"
        refuse_while_dolphin_is_open
    fi
    if [ "$1" = bind ] && [ ! -e "$backup" ]; then
        cp -p "$file" "$backup"
    fi
    chmod --reference="$file" "$temporary"
    mv "$temporary" "$file"
}

only_ours_left() {
    grep -q '<gui name="dolphin" version="1">' "$file" && ! grep -qE '<(Action|Menu|MenuBar|ToolBar|State)[ />]' "$file"
}

case $command in
bind)
    if [ -e "$file" ]; then
        write_edited bind
        exit 0
    fi
    refuse_while_dolphin_is_open
    mkdir -p "$(dirname "$file")"
    cat >"$file" <<'XML'
<!DOCTYPE gui>
<gui name="dolphin" version="1">
 <ActionProperties scheme="Default">
  <Action name="toggle_selection_mode" shortcut="; "/>
  <Action name="servicemenu_keek.desktop::quicklook" shortcut="Space"/>
 </ActionProperties>
</gui>
XML
    ;;
unbind)
    [ -e "$file" ] || exit 0
    write_edited unbind
    if only_ours_left; then
        rm -f "$file"
    fi
    rm -f "$backup"
    ;;
status)
    if [ -e "$file" ] && edit status >/dev/null; then
        echo bound
    else
        echo unbound
    fi
    ;;
check)
    refuse_while_dolphin_is_open
    ;;
-h | --help)
    usage
    ;;
*)
    usage >&2
    exit 1
    ;;
esac
