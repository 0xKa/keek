# Development

How to build, run and test Keek. The Dolphin side is in [dolphin-shortcut.md](dolphin-shortcut.md).

## Requirements

Fedora 44 with Plasma 6.8 and Qt 6.11. The build needs these packages:

```bash
sudo dnf install cmake ninja-build extra-cmake-modules \
  qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtpdf-devel qt6-qtmultimedia-devel \
  kf6-kwindowsystem-devel kf6-kio-devel kf6-syntax-highlighting-devel \
  kf6-kcoreaddons-devel kf6-kdbusaddons-devel
```

The tests also need `dbus-run-session`, `kwin_wayland`, `dolphin` and `qdbus-qt6`. They ship with a Plasma desktop.

## Build and test

Run both from the repository root:

```bash
cmake --workflow --preset build
cmake --workflow --preset test
```

- **`build`** configures and compiles into `build/`. The programs land in `build/bin/`.
- **`test`** does the same and then runs every automated test.
- **Configure prints `fatal: no upstream configured for branch`** on a branch that has not been pushed. It comes from a KDE build module looking for translations and is harmless.

To run one group of tests, or the QML linter:

```bash
ctest --preset dev -L unit
ctest --preset dev -L offscreen
ctest --preset dev -L session
ctest --preset dev -L dolphin
cmake --build --preset dev --target all_qmllint
```

No C++ formatter is set up yet.

## Install and uninstall

Everything installs under `~/.local`, with no `sudo`:

```bash
cmake --install build
keek quit
```

The install writes four files:

| File | Purpose |
|---|---|
| `~/.local/bin/keek` | The command Dolphin runs. |
| `~/.local/bin/keekd` | The daemon. |
| `~/.local/share/kio/servicemenus/keek.desktop` | The "Quick Look" action for Dolphin. |
| `~/.local/share/dbus-1/services/io.github._0xKa.keek.service` | Lets D-Bus start `keekd` on the first call. |

- **`keek quit` after an install** stops the old daemon. The next call starts the new one.
- **The first install on a login session** may need D-Bus to reread its service files: `busctl --user call org.freedesktop.DBus /org/freedesktop/DBus org.freedesktop.DBus ReloadConfig`.
- **Binding Space** is a separate, manual step. See [dolphin-shortcut.md](dolphin-shortcut.md).

To remove the four files:

```bash
keek quit
cmake --build --preset dev --target uninstall
```

## Layout

| Path | What it holds |
|---|---|
| `src/core/` | The `keekcore` library: the controller, the popup window module, the renderers and the QML interface. |
| `src/daemon/` | `keekd`: start-up, the D-Bus service and the JSON log. |
| `src/cli/` | `keek`: the command-line client. It links only Qt Core and Qt D-Bus. |
| `src/keek_config.h.in` | The version, application ID and D-Bus names, filled in by CMake. |
| `data/` | The service-menu file and the D-Bus service file, filled in by CMake. |
| `scripts/` | The screenshot page. |
| `tests/` | The automated tests, the sample files in `fixtures/` and the reference screenshots in `reference/`. |

The application ID is set once, as `KEEK_APP_ID` in the top-level `CMakeLists.txt`. The D-Bus service name and interface name are derived from it.

- **`Controller`** holds the state: the files being shown, the renderer chosen for the first one, the last error, and whether the last request carried an activation token. It is the object the QML binds to.
- **`PopupWindow`** is the one place that creates and shows the window. A later move to a layer-shell overlay changes this class and nothing else.
- **`RendererRegistry`** picks a renderer for a file from its MIME type.
- **`DBusService`** exposes the controller on the session bus. It contains no logic of its own.

`PopupWindow` loads the QML in `load()`, not in its constructor. The QML reads controller properties as soon as it loads, so the controller must be fully constructed first.

## Renderers

A renderer is a `Renderer` value: an ID, the name of a QML view, and a function that says whether it accepts a MIME type. The registry asks them in order and takes the first that accepts.

| ID | View | Accepts | Notes |
|---|---|---|---|
| `image` | `ImageView.qml` | Every type Qt's image reader can decode on this system, except PDF. | Shown at natural size, and shrunk only when larger than the popup. Vector images are scaled to fit. |
| `text` | `TextView.qml` | Every type that inherits `text/plain`. | Highlighted by file name. Only the first 512 KiB is loaded. A file with NUL bytes is reported as not text. |
| `placeholder` | `PlaceholderView.qml` | Everything else. | Shows the type's description. |

Up, Down, Page Up, Page Down, Home and End scroll the text view.

To add a renderer:

1. Add a `<name>renderer.cpp` that returns the `Renderer`, and declare it in `renderer.h`.
2. Add a `<Name>View.qml` whose root is `PreviewView`. It reads the file from `controller.currentFileUrl`, and sets `ready` and `error`.
3. Register it in `RendererRegistry`'s constructor, before the placeholder, and list the files in `src/core/CMakeLists.txt`.
4. Add sample files to `tests/fixtures/`, rows to the tables in `tst_renderers.cpp` and `tst_previews.cpp`, and generate the reference screenshots.

## Commands

`keek` forwards each command to `keekd` over D-Bus.

| Command | What it does |
|---|---|
| `keek show <paths>` | Show the popup for these files. |
| `keek toggle <paths>` | Show the popup, or hide it if it is already showing the first of these files. |
| `keek hide` | Hide the popup. |
| `keek state` | Print the current state as one line of JSON. |
| `keek screenshot <file>` | Save what the popup is drawing to an image file. It fails if the popup is hidden. |
| `keek quit` | Stop the daemon, and return once its bus name is free. |

Relative paths are resolved against the directory `keek` was run from. If `XDG_ACTIVATION_TOKEN` is set, `show` and `toggle` pass it to the daemon.

Once Keek is installed, every command except `quit` starts the installed `keekd` if no daemon is running.

Exit codes:

| Code | Meaning |
|---|---|
| 0 | Success. |
| 1 | Usage error. |
| 2 | `keekd` is not running or did not answer. |
| 3 | `keekd` answered with an error. The message is on standard error. |

### State

`keek state` prints these fields:

| Field | Meaning |
|---|---|
| `visible` | Whether the popup is showing. |
| `active` | Whether the popup has keyboard focus. |
| `status` | `hidden`, `loading`, `ready` or `error`. Wait for `ready` or `error` before taking a screenshot. |
| `file` | The file being shown, or `null`. It keeps its value after the popup is hidden. |
| `files` | Every file passed to the last `show`. |
| `renderer` | The renderer for `file`: `image`, `text` or `placeholder`, or `null`. |
| `mimeType` | The MIME type of `file`, or `null`. |
| `width`, `height` | The popup's size. |
| `activationToken` | Whether the last `show` carried an activation token. |
| `lastError` | The last error, or `null`. A successful `show` clears it. |

### D-Bus

- **Service:** `io.github._0xKa.keek`
- **Object:** `/Keek`
- **Interface:** `io.github._0xKa.keek.Keek`
- **Methods:** `Show(as paths, s activationToken)`, `Toggle(as paths, s activationToken)`, `Hide()`, `State() → s`, `Screenshot(s file)`, `Quit()`

Only one `keekd` can own the service name. A second one logs "keekd is already running" and exits with code 1.

### Logs

`keekd` writes one JSON object per line to standard error, with the fields `time`, `level`, `category` and `message`. Messages from Qt and KDE libraries go through the same format. A daemon started by D-Bus logs to the user journal: `journalctl --user -e | grep keek`.

## Trying a change

Run the daemon in one terminal and send it commands from another:

```bash
./build/bin/keek quit
./build/bin/keekd
```

```bash
./build/bin/keek show README.md
./build/bin/keek state
```

The real popup opens on the desktop. Space or Esc closes it. The first `keek quit` stops an installed daemon so that the one from `build/` can take the name.

To try the change from Dolphin, install it: `cmake --install build && keek quit`.

## The screenshot page

```bash
cmake --build --preset dev --target screenshot-page
```

This shows every file in `tests/fixtures/` in an offscreen daemon and writes `build/screenshots/index.html`, with one screenshot per file. Open it after an interface change to see every case at once. It uses Qt's plain offscreen theme, not the Plasma colours.

## Tests

| Test | Label | What it covers |
|---|---|---|
| `renderers` | `unit` | Which renderer each sample file gets, text loading (encodings, truncation, binary data) and image sizes. |
| `controller` | `offscreen` | The controller and the popup in one process: show, toggle, hide, the close keys and screenshots. |
| `previews` | `offscreen` | Every sample file shown in the popup and compared with its reference screenshot, plus switching files and scrolling. |
| `cli` | `session` | The real `keekd` and `keek` programs on a private session bus started by `dbus-run-session`. |
| `dolphin` | `dolphin` | A real Dolphin in a headless KWin session with throwaway settings. It triggers the Quick Look action with two files selected and checks what reaches the popup. |
| `appstreamtest` | none | Added by a KDE build module. It passes because there is no metadata to check yet. |

- **The tests use their own settings.** They run with `QT_QPA_PLATFORM=offscreen`, the software renderer, and `XDG_*` folders under `build/test-home/`, so the user's settings and an installed Keek do not affect them.
- **The `cli` test skips itself when run directly**, so that it can never talk to a daemon on the real session bus. Run it through `ctest`.
- **The `dolphin` test triggers the action over D-Bus.** It does not press the real Space key, and a headless session cannot show whether KWin would refuse focus at stricter settings.

### Reference screenshots

`previews` fails when more than 0.1% of the pixels differ from the reference by more than 40 levels in a channel. On failure it saves what the popup drew to `build/test-output/<file>.actual.png`.

The references depend on the fonts installed on the machine, so a font or Qt update can change them. After checking that the new output is right, regenerate them:

```bash
KEEK_UPDATE_REFERENCES=1 ctest --preset dev -R previews
```
