# Code style

The choices this project has made for C++, QML, Bash and CMake. Read it before editing a `.cpp`, `.h`, `.qml`, `.sh` or `CMakeLists.txt` file.

It lists only what is specific to Keek or has already caused a bug here. When an agent gets something wrong that a rule would have prevented, add the rule.

## Where logic goes

| Rule | Detail |
|---|---|
| State lives in `Controller` | The files being shown, the chosen renderer and the last error. Nothing else keeps a copy. |
| `DBusService` holds no logic | Each method checks its arguments, calls the controller, and turns a failure into a D-Bus error reply. |
| `PopupWindow` is the only window code | Creating, showing, focusing and grabbing the window happen there and nowhere else. |
| Views only report | A QML view draws one file and sets `ready` and `error`. It does not decide which file or which renderer. |
| Read QML state on demand | C++ asks the root item for `contentReady` and `contentError` when it needs them. Do not mirror them into C++ members; a missed change signal leaves the copy stale. |
| File work is C++ | Reading, decoding and measuring files belong in a C++ type exposed to QML, such as `TextFile` and `ImageInfo`. QML does not read files. |

## C++

| Rule | Detail |
|---|---|
| No comments | Names and small functions carry the meaning. This includes licence headers. |
| Never load QML from a constructor | QML reads the controller's properties the moment it loads. Load it in a separate step once the object is fully built, as `PopupWindow::load()` does. This crashed the daemon twice. |
| Declare members in dependency order | A member used by another member's constructor comes first. In `Controller`, `m_window` is last. |
| Qt macros, not keywords | `Q_SIGNALS`, `Q_SLOTS` and `Q_EMIT`, never `signals`, `slots` or `emit`. |
| String literals | `u"text"_s` for a `QString`, `"text"_L1` for comparisons and JSON keys, `QStringLiteral(MACRO)` when the text comes from a macro. No bare `"text"` where a `QString` is expected. |
| Ownership | `std::unique_ptr` for an object with no Qt parent, a parent for a `QObject` that has one. No bare `new` without one of the two. A `KJob` is the exception: it deletes itself when it finishes. |
| Properties for QML | Mark each `Q_PROPERTY` `FINAL`, and give every property that can change a `NOTIFY` signal. |
| QML types | Register with `QML_ELEMENT` in the header and list the files in `src/core/CMakeLists.txt`. Add `QML_UNCREATABLE` when QML must not create the type. |
| Errors | A function that can fail returns `bool` and fills a `QString *error`. No exceptions. |
| Translation | Text shown in the popup goes through `tr()`. Command-line output, D-Bus error replies and log messages stay in plain English, so scripts and agents can match them. |
| Formatting | `.clang-format` holds the style. Run `cmake --build --preset dev --target clang-format` before committing C++ changes. |
| Logging | `qCInfo(lcKeek)` and `qCWarning(lcKeek)`. Never `qDebug()` or `std::cout` in the daemon; they break the JSON log. |
| File-local helpers | Put them in an anonymous namespace, not `static`. |
| Headers | `#pragma once`. Include the file's own header first, then project headers, KDE, Qt, and the standard library. |
| `keek` stays small | The command-line tool links only Qt Core and Qt D-Bus. Do not add Qt Gui or `keekcore` to it. |

## QML

| Rule | Detail |
|---|---|
| Lint stays clean | `cmake --build --preset dev --target all_qmllint` prints no warnings. Fix a warning; do not suppress it. |
| Typed properties | `required property Controller controller`, not `var` or `QtObject`. Cast a `Loader` item with `as` before using it. |
| Qualify by `id` | Write `root.controller` and `view.width`, never a bare name from an outer scope. |
| Views extend `PreviewView` | The root of every view is `PreviewView`. It reads the file from `controller.currentFileUrl`. |
| No popup keys in views | Space, Esc, Enter and the arrows without modifiers belong to `Popup.qml`. A view handles only its own keys, such as Ctrl + ↑ or Page Down, and leaves the rest unaccepted. |
| Ignore key repeat on close and open | The Space, Esc and Enter handlers return early when `event.isAutoRepeat` is set. Held arrows keep moving. |
| Sizes from `Metrics` | Margins, heights, font sizes and opacities come from the `Metrics` singleton. No inline numbers for layout; add a named value there instead. |
| Shared pieces | Use `CenteredMessage` for a centred note or error. Do not copy its `Text` block. |
| Colours from the palette | Use `palette.window`, `palette.base` and `palette.text`. No hard-coded colours, so light and dark themes both work. |
| No stale content | Set `cache: false` on an `Image`, and keep the `Loader` inactive while the popup is hidden. |
| Text for the user | Wrap it in `qsTr()`. |

## Bash

| Rule | Detail |
|---|---|
| Start with `set -euo pipefail` | Leave out `-e` only in a script that reports failures itself, as `tests/dolphin-session.sh` does. |
| Never touch the user's session | A script that starts `keekd` or Dolphin sets throwaway `XDG_*` folders and a private bus with `dbus-run-session`. |
| Keep throwaway paths short | A Wayland socket path cannot exceed 108 bytes. Use `mktemp -d /tmp/keek-XXXXXX`. |
| Poll, do not sleep | Wait for a condition with a bounded loop on `keek state`. A fixed `sleep` is either slow or flaky. |
| No comments | Same as the C++ rule. |

## CMake

| Rule | Detail |
|---|---|
| One source for names | The application ID is `KEEK_APP_ID` in the top-level file. Files that need it are generated with `configure_file`. |
| New tests go through `keek_add_test` | Then add the test with a label: `unit`, `offscreen`, `session` or `dolphin`. Give it `KEEK_TEST_ENVIRONMENT` so it cannot read the user's settings. |
| Shared test helpers | `tests/fixtures.h` gives the path of a sample file and copies the sample files with a fixed date, and `tests/imagecompare.h` compares screenshots. Do not redefine them in a test. |
| Link only what is used | Do not add a Qt or KDE component to `find_package` until code needs it. |
