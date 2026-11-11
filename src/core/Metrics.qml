pragma Singleton
import QtQuick

QtObject {
    readonly property int margin: 24
    readonly property int titleHeight: 40
    readonly property int titleFontSize: 14
    readonly property int messageFontSize: 16
    readonly property real mutedOpacity: 0.7

    readonly property int textPaddingX: 16
    readonly property int textPaddingY: 12
    readonly property int tabWidthInCharacters: 4
    readonly property int linesPerScrollStep: 1
    readonly property int linesKeptOnPageScroll: 2
    readonly property int noticeHeight: 28

    readonly property int scrollBarWidth: 6
    readonly property int scrollBarMargin: 4
    readonly property int scrollBarMinimumLength: 24
    readonly property real scrollBarOpacity: 0.35
}
