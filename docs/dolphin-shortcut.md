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
2. **Set the two shortcut lines.** If `dolphinui.rc` exists, back it up and merge the two `Action` lines into its `ActionProperties` element, leaving everything else alone. If it does not exist, the installer cannot write a two-line file, because the file also carries the layout and its version. How to handle that case is still open; the options are to ask the user to bind Space by hand, or to generate the file from Dolphin's own shipped layout.
3. **Ask the user to restart Dolphin.** See "When Dolphin reads the file" below.
4. **Uninstall in reverse:** remove the Quick Look line, restore Space on `toggle_selection_mode` by removing its line, and delete the service-menu file.

Edit `dolphinui.rc` only while Dolphin is closed. A running Dolphin holds the shortcuts in memory and writes the file when they change, so it can overwrite the edit. This has not been tested.

### Binding by hand

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

## What the real Space-key test covered

Run by hand in the user's Dolphin on 11 November 2026, with a stand-in command that logged each call:

- **In the log:** ten calls, each with one path and the activation token. They cover files, a folder, and the current folder when nothing was selected.
- **Reported by the user, not visible in the log:** Space worked in a second window, the binding survived a Dolphin restart, and typing a space while renaming, in the location bar and in the search bar did not trigger the action.
- **Not covered by hand:** several files selected at once. The `dolphin` test covers it.
