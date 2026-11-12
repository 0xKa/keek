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

To format the C++ code, which needs `clang-format` from the `clang-tools-extra` package:

```bash
cmake --build --preset dev --target clang-format
```

## Install and uninstall

Everything installs under `~/.local`, with no `sudo`. `keekd` also writes `~/.config/keekrc` the first time a mute button is pressed.

Build first, close every Dolphin window, then run:

```bash
scripts/install.sh               # install and bind Space in Dolphin
scripts/install.sh --autostart   # also start keekd at login
scripts/install.sh --no-shortcut # leave Dolphin's shortcuts alone
scripts/uninstall.sh             # remove everything and give Space back
```

`install.sh` does these steps in order:

1. **Refuses to go on while a Dolphin window is open**, unless `--no-shortcut` is given. A running Dolphin can overwrite the shortcut change.
2. **Runs `cmake --install build`**, which writes the five files below.
3. **Asks D-Bus to reread its service files**, so the first `keek` call can start the new `keekd`, and stops a running daemon with `keek quit`.
4. **Binds Space** with `scripts/dolphin-shortcut.sh bind`. The details are in [dolphin-shortcut.md](dolphin-shortcut.md).
5. **With `--autostart`, copies the autostart entry** to `~/.config/autostart/keekd.desktop`.

| File | Purpose |
|---|---|
| `~/.local/bin/keek` | The command Dolphin runs. |
| `~/.local/bin/keekd` | The daemon. |
| `~/.local/share/kio/servicemenus/keek.desktop` | The "Quick Look" action for Dolphin. |
| `~/.local/share/dbus-1/services/io.github._0xKa.keek.service` | Lets D-Bus start `keekd` on the first call. |
| `~/.local/share/keek/keekd.desktop` | The autostart entry. Plasma reads only `~/.config/autostart/`, which is outside the install prefix, so `install.sh --autostart` copies it there. |

- **Starting at login is optional.** Without it, D-Bus starts `keekd` on the first Space press, which makes that one press slower. If D-Bus has already started a daemon, the one from autostart logs "keekd is already running" and exits.
- **`uninstall.sh` reverses all of it:** it stops the daemon, unbinds Space, removes the autostart copy and `~/.config/keekrc`, and runs the `uninstall` target, which reads `build/install_manifest.txt`. It too refuses while a Dolphin window is open, if Space is still bound.
- **Both scripts read the prefix from `build/`.** Set `KEEK_BUILD_DIR` to use another build folder.
- **During development**, `cmake --install build && keek quit` is enough to try a new build. It does not touch Dolphin's shortcuts.

## Layout

| Path | What it holds |
|---|---|
| `src/core/` | The `keekcore` library: the controller, the popup window module, the renderers and the QML interface. |
| `src/daemon/` | `keekd`: start-up, the D-Bus service and the JSON log. |
| `src/cli/` | `keek`: the command-line client. It links only Qt Core and Qt D-Bus. |
| `src/keek_config.h.in` | The version, application ID and D-Bus names, filled in by CMake. |
| `data/` | The service-menu file, the D-Bus service file and the autostart entry, filled in by CMake. |
| `scripts/` | The install and uninstall scripts, the Dolphin shortcut script and the screenshot page. |
| `tests/` | The automated tests, the sample files in `fixtures/` and the reference screenshots in `reference/`. |

The application ID is set once, as `KEEK_APP_ID` in the top-level `CMakeLists.txt`. The D-Bus service name and interface name are derived from it.

- **`Controller`** holds the state: the list of files to move through, the current one, its renderer, the last error, and whether the last request carried an activation token. It is the object the QML binds to.
- **`folderlisting`** builds the list the arrow keys move through, in Dolphin's default order.
- **`PopupWindow`** is the one place that creates and shows the window. A later move to a layer-shell overlay changes this class and nothing else.
- **`RendererRegistry`** picks a renderer for a file from its MIME type.
- **`DBusService`** exposes the controller on the session bus. It contains no logic of its own.

`PopupWindow` loads the QML in `load()`, not in its constructor. The QML reads controller properties as soon as it loads, so the controller must be fully constructed first.

## Renderers

A renderer is a `Renderer` value: an ID, the name of a QML view, and a function that says whether it accepts a MIME type. The registry asks them in order and takes the first that accepts.

| ID | View | Accepts | Notes |
|---|---|---|---|
| `image` | `ImageView.qml` | Every type Qt's image reader can decode on this system, except PDF. | Shown at natural size, and shrunk only when larger than the popup. Vector images are scaled to fit. |
| `pdf` | `PdfView.qml` | PDF documents. | Every page in one scrolling list, as wide as the popup allows but never above 100% zoom (72 points to 96 pixels). Pages are drawn on white paper in both themes. |
| `text` | `TextView.qml` | Every type that inherits `text/plain`. | Highlighted by file name through `TextHighlighter`, which sets the text and the highlighting together. Each line is laid out right to left or left to right by its first letter that has a direction, so Arabic and Hebrew lines sit on the right. Only the first 512 KiB is loaded. A file with NUL bytes is reported as not text. |
| `audio` | `AudioView.qml` | Every `audio/*` type that is not text. | Qt Multimedia. Shows the cover from Dolphin's thumbnailer or the icon, with the title, artist and album when Qt can read them. Starts playing with sound. |
| `video` | `VideoView.qml` | Every `video/*` type. | Qt Multimedia. Starts playing without sound. Clicking the picture plays or pauses. |
| `folder` | `FolderView.qml` | Folders. | Shows how many files and folders it holds, its total size, its modification date and its first eight entries. Hidden entries are left out of the counts and the list, but not the size. |
| `fallback` | `FallbackView.qml` | Everything else, a missing file and a file Keek may not read. | Shows the thumbnail Dolphin would show, or the file's icon when there is none, with the type, size and modification date. When another renderer gave up on the file, it also shows why. |

To add a renderer:

1. Add a `<name>renderer.cpp` that returns the `Renderer`, and declare it in `renderer.h`.
2. Add a `<Name>View.qml` whose root is `PreviewView`. It reads the file from `controller.currentFileUrl`, sets `ready` and `error`, and sets `preferredWidth` and `preferredHeight` once it knows them. See "Popup size and placement" below.
3. Register it in `RendererRegistry`'s constructor, before the fallback, and list the files in `src/core/CMakeLists.txt`.
4. Add sample files to `tests/fixtures/`, rows to the tables in `tst_renderers.cpp` and `tst_previews.cpp`, and generate the reference screenshots.

### Popup size and placement

The popup sizes itself to the file. Each view sets `preferredWidth` and `preferredHeight`, the space its content wants below the title, and `Popup.qml` turns that into the window size. `PopupWindow` applies it.

| View | Size it asks for |
|---|---|
| Image | The image at its natural size, shrunk to fit the space available. A vector image is drawn with its longer side at 512 pixels. |
| PDF | The first page whole, at up to 100% zoom. |
| Text | As wide as the longest line, up to 100 columns, and as tall as the text. |
| Video | The video's resolution, shrunk to fit, plus the control bar. |
| Audio, folder, fallback | Their summary block plus the margins. |

- **Limits:** the popup is at least 480 × 320 and at most 85% of the screen's available area. The view is told the space it can use through its `available` property.
- **Before the size is known:** the popup uses 960 × 640. An error is sized to its message.
- **Showing waits for the size.** `PopupWindow` keeps a hidden popup hidden until the root's `sized` is true, for at most 150 ms, so it opens at its final size. `keek state` already reports `visible` as true during the wait.
- **KWin places the popup.** It centres a new window and keeps it centred when it resizes. A resize sent before the first frame is drawn can lose the centre, so `PopupWindow` holds resizes until then. When KWin answers with the old size, `PopupWindow` asks again, up to 3 times.
- **The offscreen screen is 1920 × 1080.** The tests and the screenshot page use `tests/offscreen.json`, so the sizes do not depend on Qt's default offscreen screen.

### Animation

- **Opening and closing are KWin's.** Plasma's Scale effect, on by default, already animates the popup as it appears and disappears. Keek adds nothing of its own there, so a close is never delayed.
- **The preview fades in.** When the popup opens, when the arrows move to another file and when a renderer gives up and the fallback takes over, the preview fades in over 120 ms, once its size is known. The title and errors are not faded.
- **Plasma's animation speed applies.** `DesktopSettings` reads `AnimationDurationFactor` from the `[KDE]` group of `kdeglobals` and follows changes to it. The fade lasts 120 ms times that factor, and 0 turns it off.
- **Resizing is not animated.** Every resize on Wayland is a round trip to KWin, and an early one can lose the centre, so the popup jumps to its new size.
- **`ready` waits for the fade.** `keek state` reports `loading` until the preview is fully shown, so a screenshot taken at `ready` never catches it half faded.

### Errors

A view sets `error` to one sentence that says what went wrong. `Popup.qml` then hides the view and draws the file's icon, type, size and date with the message and a warning icon below, sized to fit. `FileOverview.qml` draws this block, and the fallback view uses it too.

| Case | Who reports it | Message |
|---|---|---|
| The file does not exist | `Controller` | This file does not exist. It may have been moved or deleted. |
| No read permission | `Controller` | You do not have permission to read this file. (For a folder: to open this folder.) |
| An image that cannot be decoded | `ImageView` | This image could not be decoded. It may be damaged or in a format that is not supported. |
| A damaged PDF | `PdfView` | This PDF could not be opened. It may be damaged. |
| A PDF with a password | `PdfView` | This PDF is protected by a password. |
| A text file that cannot be read | `TextView` | This file could not be read: the system's reason. |
| A text file with NUL bytes | `TextView` | This file holds binary data, not text. |

A status of `error` in `keek state` means one of these is showing.

Audio and video that cannot be played are not errors. The view calls `Controller::useFallback(reason)`, the fallback view shows the file with the reason and an information icon, and `keek state` reports the reason as `notice`.

### Thumbnails and icons

- **`Thumbnail`** asks `KIO::PreviewJob` for the same thumbnail Dolphin shows. It uses the plugins enabled in Dolphin's settings (`dolphinrc`, `[PreviewSettings] Plugins`), or KIO's defaults when Dolphin has none.
- **`FileIcon`** draws an icon from the icon theme. When no platform theme sets one, as in the offscreen tests, `PopupWindow` falls back to Breeze from the system icon folders.
- **`FileDetails`** gives a file's icon name, type, size and modification date, read through `KFileItem`.
- **`FileSummary.qml`** puts the thumbnail or icon above a few lines of details.

### Audio and video

`MediaView.qml` holds the player and the control bar that `AudioView.qml` and `VideoView.qml` share.

- **Mute is remembered** separately for audio and video, in `~/.config/keekrc`:

  ```ini
  [Media]
  AudioMuted=false
  VideoMuted=true
  ```

  These are the defaults when the file does not exist. `MediaSettings` reads and writes them.
- **`KEEK_AUTOPLAY=0`** in `keekd`'s environment stops playback from starting on its own. The player pauses on the first frame instead. The tests and the screenshot page set it, so they never make a sound.
- **Files that cannot be played fall back.** When the player reports an error, finds no picture in a video or no sound in an audio file, or no video frame arrives within 3 seconds, the view calls `Controller::useFallback()` and the popup shows the fallback view with the reason. This covers H.265 video on a Fedora install without RPM Fusion's codecs.
- **A video is `ready`** once its first frame has arrived, so a screenshot taken then shows a picture.
- **Qt reads tags only from the container.** FLAC and MP3 titles show; Ogg Vorbis and Opus titles do not, because they sit on the stream.

### Folder size

`FolderInfo` adds up the size of every file under the folder on a Qt Concurrent worker thread, without following symbolic links. The popup shows the running total every 200 ms and the status stays `loading` until the count is done. Showing another file cancels the count.

## Keys

| Key | What it does |
|---|---|
| ↑ or ← | Show the previous file. |
| ↓ or → | Show the next file. |
| Ctrl + ↑ or ↓ | Scroll one line. |
| Page Up, Page Down | Scroll one page. |
| Home, End | Go to the top or the bottom. |
| K | Play or pause audio or video. |
| M | Mute or unmute audio or video. |
| Shift + ← or → | Go back or forward 5 seconds in audio or video. |
| Enter | Close the popup and open the file in its default application. |
| Space or Esc | Close the popup. |

`Popup.qml` handles the arrows without modifiers, Enter, Space and Esc. Every other key goes to the view first. The text and PDF views scroll with `KeyScroller.qml` and show their position with `ScrollMarker.qml`.

### Which files the arrows move through

- **One file shown:** every file and folder next to it, in Dolphin's default order: folders first, then names in natural order (`file2` before `file10`), ignoring case. Hidden files are left out unless the shown file is hidden.
- **Several files shown:** only those files, in the order they were given.
- **At either end** the arrow does nothing. The list does not wrap.
- **The list is built once per `show`.** Files added to the folder while the popup is open are not in it.

Dolphin's selection does not follow. Dolphin offers no way to read or set it from outside.

### Opening a file

Enter asks KWin for an activation token, so the application that opens gets focus, and then opens the file with `KIO::OpenUrlJob`. Executables are not run. Tests replace the opener with `Controller::setFileOpener`, so they never start an application.

## Commands

`keek` forwards each command to `keekd` over D-Bus.

| Command | What it does |
|---|---|
| `keek show <paths>` | Show the popup for these files. |
| `keek toggle <paths>` | Show the popup, or hide it if it is already showing the first of these files. |
| `keek hide` | Hide the popup. |
| `keek next` | Show the next file, as ↓ does. It fails if the popup is hidden. |
| `keek previous` | Show the previous file, as ↑ does. It fails if the popup is hidden. |
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
| `index` | The position of `file` in the list the arrows move through, from 0, or `null`. |
| `count` | How many files that list holds. |
| `renderer` | The renderer for `file`: `image`, `pdf`, `text`, `audio`, `video`, `folder` or `fallback`, or `null`. |
| `mimeType` | The MIME type of `file`, or `null`. |
| `notice` | Why the fallback view is showing a file another renderer gave up on, such as a video that cannot be played, or `null`. |
| `width`, `height` | The popup's size. |
| `activationToken` | Whether the last `show` carried an activation token. |
| `lastError` | The last error, or `null`. A successful `show` clears it. |

### D-Bus

- **Service:** `io.github._0xKa.keek`
- **Object:** `/Keek`
- **Interface:** `io.github._0xKa.keek.Keek`
- **Methods:** `Show(as paths, s activationToken)`, `Toggle(as paths, s activationToken)`, `Hide()`, `Next()`, `Previous()`, `State() → s`, `Screenshot(s file)`, `Quit()`

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
| `listing` | `unit` | The order of the list the arrows move through, and which files it leaves out. |
| `controller` | `offscreen` | The controller and the popup in one process: show, toggle, hide, moving between files, Enter, the close keys and screenshots. |
| `previews` | `offscreen` | Every sample file shown in the popup and compared with its reference screenshot, plus the popup's size, the fade-in, switching files, scrolling, the media keys and a thumbnail. |
| `cli` | `session` | The real `keekd` and `keek` programs on a private session bus started by `dbus-run-session`. |
| `dolphin` | `dolphin` | A real Dolphin in a headless KWin session with throwaway settings. It triggers the Quick Look action with two files selected, checks what reaches the popup, and checks that KWin keeps the popup centred as it changes size. It also binds Space on a fresh account and checks that Dolphin merges the stub, that selection mode loses Space and gets it back, and that edits wait for Dolphin to close. |
| `shortcut` | `unit` | `scripts/dolphin-shortcut.sh` on a sample `dolphinui.rc` and on variants of it: binding, unbinding, a missing file, a custom shortcut and a layout it must refuse. |
| `appstreamtest` | none | Added by a KDE build module. It passes because there is no metadata to check yet. |

- **The tests use their own settings.** They run with `QT_QPA_PLATFORM=offscreen`, the software renderer, and `XDG_*` folders under `build/test-home/`, so the user's settings and an installed Keek do not affect them.
- **The `cli` test skips itself when run directly**, so that it can never talk to a daemon on the real session bus. Run it through `ctest`.
- **The `dolphin` test triggers the action over D-Bus.** It does not press the real Space key, and a headless session cannot show whether KWin would refuse focus at stricter settings.

### CI

GitHub Actions runs `.github/workflows/ci.yml` on every push to `main` and every pull request, in a `fedora:44` container. A newer push cancels the run still going for the same branch. It installs the build packages, then:

1. Builds with `cmake --workflow --preset build`.
2. Runs the `unit` tests.
3. Runs the `controller` and `cli` tests.
4. Runs the QML linter.
5. Runs the `clang-format` target and fails if it changed any file.
6. Renders the screenshot page and attaches it to the run as the `screenshots` artifact. Download it from the run's summary page to see how a change looks before merging it.

The `previews` and `dolphin` tests do not run in CI. The reference screenshots depend on this machine's fonts, and the `dolphin` test does not run reliably in the container. Run them locally when a change touches what they cover.

### Reference screenshots

`previews` fails when more than 0.1% of the pixels differ from the reference by more than 40 levels in a channel. On failure it saves what the popup drew to `build/test-output/<file>.actual.png`.

The offscreen platform cannot draw video frames, so the video reference shows the control bar over an empty picture. A real or headless KWin session draws them; the commands under "Trying a change" work there too.

Besides the sample files, it shows three cases made at run time: `missing.png`, which does not exist, and `unreadable.txt` and `locked/`, which have no permissions. The last two are skipped when run as root. The test shows copies of the sample files, all dated 2 January 2026 at 03:04, and runs with `TZ=UTC`, so the dates in the popup do not change between checkouts. A thumbnail depends on the plugins installed, so no reference screenshot shows one; `comic.cbz` is checked by a separate test that only asks whether a thumbnail appeared.

The references depend on the fonts and the Breeze icons installed on the machine, so a font or Qt update can change them. After checking that the new output is right, regenerate them:

```bash
KEEK_UPDATE_REFERENCES=1 ctest --preset dev -R previews
```
