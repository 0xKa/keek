# The Dolphin shortcut for Quick Look

Keek is triggered by a Dolphin service-menu action with Space bound to it. This page records how that binding works, what Dolphin saves, and what an installer has to do to set it up for a user.

Checked on 11 November 2026 with Fedora 44, Plasma 6.8.2, Dolphin 26.08.2 and KDE Frameworks 6.31. Facts marked "from source" were read from the main branches of Dolphin and KIO, not from the 26.08.2 release, so recheck them if behaviour differs.

## The service-menu file

The file is generated from `data/keek.desktop.in` and installed by `cmake --install build`. It lives at `~/.local/share/kio/servicemenus/keek.desktop` and must be executable:

```ini
[Desktop Entry]
Type=Service
MimeType=all/all;
Actions=quicklook

[Desktop Action quicklook]
Name=Quick Look
Icon=view-preview
Exec=/home/<user>/.local/bin/keek show %F
```

- **The names are fixed.** Dolphin names the action `servicemenu_keek.desktop::quicklook`, built from the file name and the action key. The saved shortcut is tied to that name, so renaming the file or the action key drops every user's binding.
- **`all/all` covers files and folders.**
- **`desktop-file-validate` rejects the file.** It does not know the service-menu format. Ignore its errors.

## What Dolphin passes to the command

- **Selected items:** their local paths, through `%F`. Several selected items arrive as several arguments. The `dolphin` test checks this with two files.
- **Selection cleared:** the folder being viewed. Once a selection has been made and then cleared, Dolphin substitutes the current folder (from source, and seen in the real test). Keek cannot tell this apart from that folder being selected in its parent.
- **Nothing selected yet:** no call at all. In a window where nothing has been selected since it opened, the action does not run. This was seen in the isolated session.
- **Activation token:** the environment variable `XDG_ACTIVATION_TOKEN` is set on every call triggered by a real key press. Ten out of ten calls in the real test had it.

## Where the shortcut is saved

Dolphin saves it in `~/.local/share/kxmlgui5/dolphin/dolphinui.rc`, inside the `ActionProperties` element:

```xml
<ActionProperties scheme="Default">
 ...
 <Action name="toggle_selection_mode" shortcut="; "/>
 <Action name="servicemenu_keek.desktop::quicklook" shortcut="Space"/>
</ActionProperties>
```

- **The first line unbinds selection mode.** Dolphin ships with Space bound to `toggle_selection_mode` ("Select Files and Folders"). The value `"; "` is an empty main shortcut and an empty alternate, which overrides that default.
- **The second line binds Quick Look.**
- **The file does not exist on a fresh account.** Dolphin creates it on the first customisation, as a full copy of its menu and toolbar layout with a `version` attribute on the root element (142 lines, version 49 here).

## What an installer has to do

1. **Install the service-menu file** and make it executable. `cmake --install build` already does this.
2. **Set the two shortcut lines.** If `dolphinui.rc` exists, back it up and merge the two `Action` lines into its `ActionProperties` element, leaving everything else alone. If it does not exist, write a stub with `version="1"` that holds only the `ActionProperties` element. See "A fresh account" below.
3. **Make sure Dolphin is closed.** See "When Dolphin reads the file" below.
4. **Uninstall in reverse:** remove the Quick Look line, restore Space on `toggle_selection_mode` by removing its line, and delete the service-menu file.

Edit `dolphinui.rc` only while Dolphin is closed. A running Dolphin holds the shortcuts in memory and writes the file when they change, so it can overwrite the edit.

### A fresh account

From the KXMLGUI source (`kxmlguiversionhandler.cpp`), checked in the `dolphin` test: when the local file has a lower `version` than the layout Dolphin ships, KXMLGUI loads the shipped layout, copies the local `ActionProperties` entries into it, and writes the result over the local file. So a stub like this is enough:

```xml
<!DOCTYPE gui>
<gui name="dolphin" version="1">
 <ActionProperties scheme="Default">
  <Action name="toggle_selection_mode" shortcut="; "/>
  <Action name="servicemenu_keek.desktop::quicklook" shortcut="Space"/>
 </ActionProperties>
</gui>
```

On the first start, Dolphin turns it into its full 142-line file at version 49, with the two lines kept. Dolphin writes them back in its own order, Quick Look first.

### `scripts/dolphin-shortcut.sh`

This script does steps 2 and 4 for the installer:

```bash
scripts/dolphin-shortcut.sh bind      # give Space to Quick Look
scripts/dolphin-shortcut.sh unbind    # give it back to selection mode
scripts/dolphin-shortcut.sh status    # prints bound or unbound
scripts/dolphin-shortcut.sh check     # fails while a Dolphin window is open
```

Each takes an optional path to the file, which defaults to `~/.local/share/kxmlgui5/dolphin/dolphinui.rc` (or the same path under `XDG_DATA_HOME`).

- **Only the `ActionProperties` element is edited.** The `toggle_selection_mode` entries in the menus and the toolbar look the same, so the script edits lines only inside that element. It expects one element per line, as Dolphin writes them. If the file has another layout, it exits with code 2 and leaves the file alone.
- **Selection mode keeps its other keys.** If the user gave it more than one shortcut, only Space is removed. Unbinding removes the line only when it is exactly `shortcut="; "`, the line `bind` writes, so a shortcut the user set is kept.
- **The first change makes a backup**, `dolphinui.rc.before-keek`. `unbind` deletes it once it has undone the change.
- **A missing file gets the stub**, and `unbind` deletes the stub if Dolphin has not started since.
- **Nothing is written when nothing would change**, so binding twice is harmless.
- **A change is refused while Dolphin is open**, with exit code 4. A Dolphin window registers `org.kde.dolphin-<pid>` with a `/dolphin/Dolphin_1` object on the session bus. `dolphin --daemon` has no window and never writes the file, so it does not count.

### Binding by hand

`scripts/install.sh` does this unless it is given `--no-shortcut`. To do it in Dolphin instead:

1. Close every Dolphin window and open Dolphin again.
2. Open Configure Keyboard Shortcuts (Ctrl+Alt+,) and search for "Quick Look".
3. Set its shortcut to Space and accept the prompt to take it from "Select Files and Folders".

## What to do with selection mode

Leave it unbound. The installer should say that Space has moved and that selection mode is still in the menu.

Do not move it to Ctrl+Space. Dolphin's file view uses Ctrl+Space itself to toggle the selection of the current item, which is how files that are not adjacent get selected from the keyboard (from source). Ctrl+Space is also the usual key for switching input methods.

## When Dolphin reads the file

From source:

- **Each window keeps its own copy.** A Dolphin window reads the service-menu actions, command line included, when it sets up, and again when the context-menu settings are applied. A window opened before the file was installed has no Quick Look action.
- **A missing file can erase the binding.** When the shortcuts dialog is opened, Dolphin removes saved shortcuts for service-menu actions that no longer exist.

For development this means the `Exec` line should keep pointing at one stable path, `~/.local/bin/keek`. Installing a new build over that path takes effect at once; changing the `Exec` line needs a Dolphin restart.

## Triggering the action without a keyboard

For tests and agents, Dolphin's main-window D-Bus interface can list and trigger the action:

```bash
svc=$(busctl --user list --no-legend | grep -o 'org.kde.dolphin-[0-9]*' | head -1)
qdbus-qt6 "$svc" /dolphin/Dolphin_1 org.kde.KMainWindow.actions
qdbus-qt6 "$svc" /dolphin/Dolphin_1 org.kde.KMainWindow.activateAction "servicemenu_keek.desktop::quicklook"
```

The action does not appear under `/dolphin/Dolphin_1/actions/`, unlike Dolphin's built-in actions, so `activateAction` is the way in. The `dolphin` test in `tests/dolphin-session.sh` does this in an isolated headless session; it does not exercise the real Space key.

The built-in actions show their shortcut as a list of key codes, which is how the `dolphin` test checks that Space left selection mode and came back:

```bash
busctl --user get-property "$svc" /dolphin/Dolphin_1/actions/toggle_selection_mode org.qtproject.Qt.QAction shortcut
```

`(ai) 4 32 0 0 0` is Space, the default. `(ai) 4 0 0 0 0` is no shortcut.

## What the real Space-key test covered

Run by hand in the user's Dolphin on 11 November 2026, with a stand-in command that logged each call:

- **In the log:** ten calls, each with one path and the activation token. They cover files, a folder, and the current folder when nothing was selected.
- **Reported by the user, not visible in the log:** Space worked in a second window, the binding survived a Dolphin restart, and typing a space while renaming, in the location bar and in the search bar did not trigger the action.
- **Not covered by hand:** several files selected at once. The `dolphin` test covers it.
