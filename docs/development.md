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

`dbus-run-session` is also needed for the tests. It ships with a Plasma desktop.

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
ctest --preset dev -L offscreen
ctest --preset dev -L session
cmake --build --preset dev --target all_qmllint
```

No C++ formatter is set up yet.

## Layout

| Path | What it holds |
|---|---|
| `src/core/` | The `keekcore` library: the controller, the popup window module and the QML interface. |
| `src/daemon/` | `keekd`: start-up, the D-Bus service and the JSON log. |
| `src/cli/` | `keek`: the command-line client. It links only Qt Core and Qt D-Bus. |
| `src/keek_config.h.in` | The version, application ID and D-Bus names, filled in by CMake. |
| `tests/` | The automated tests. |

The application ID is set once, as `KEEK_APP_ID` in the top-level `CMakeLists.txt`. The D-Bus service name and interface name are derived from it.

- **`Controller`** holds the state: the files being shown, the last error, and whether the last request carried an activation token. It is the only C++ object the QML sees.
- **`PopupWindow`** is the one place that creates and shows the window. A later move to a layer-shell overlay changes this class and nothing else.
- **`DBusService`** exposes the controller on the session bus. It contains no logic of its own.

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
| `file` | The file being shown, or `null`. It keeps its value after the popup is hidden. |
| `files` | Every file passed to the last `show`. |
| `renderer` | The renderer in use, or `null`. For now it is always `placeholder`. |
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

`keekd` writes one JSON object per line to standard error, with the fields `time`, `level`, `category` and `message`. Messages from Qt and KDE libraries go through the same format.

## Trying a change

Run the daemon in one terminal and send it commands from another:

```bash
./build/bin/keekd
```

```bash
./build/bin/keek show README.md
./build/bin/keek state
./build/bin/keek quit
```

The real popup opens on the desktop. Space or Esc closes it. After a rebuild, run `keek quit` and start `keekd` again, or the old daemon keeps answering.

## Tests

| Test | Label | What it covers |
|---|---|---|
| `controller` | `offscreen` | The controller and the popup in one process, with `QT_QPA_PLATFORM=offscreen`: show, toggle, hide, the close keys and screenshots. |
| `cli` | `session` | The real `keekd` and `keek` programs on a private session bus started by `dbus-run-session`. |
| `appstreamtest` | none | Added by a KDE build module. It passes because there is no metadata to check yet. |

The `cli` test skips itself when it is run directly, so that it can never talk to a daemon on the real session bus. Run it through `ctest`.

## Rules for QML and C++ objects

A C++ object that creates the popup must declare its `PopupWindow` member last. Loading the QML reads properties of that object straight away, so every member those properties use has to be constructed first.
