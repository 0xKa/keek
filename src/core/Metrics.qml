pragma Singleton
import QtQuick

QtObject {
    readonly property int margin: 24
    readonly property int titleHeight: 40
    readonly property int titleFontSize: 14
    readonly property int messageFontSize: 16
    readonly property real mutedOpacity: 0.7

    readonly property int iconSize: 128
    readonly property int thumbnailSize: 256
    readonly property int detailSpacing: 6
    readonly property int detailFontSize: 13

    readonly property int sectionSpacing: 20
    readonly property int folderEntries: 8
    readonly property int entryListWidth: 360
    readonly property int entryHeight: 28
    readonly property int entryIconSize: 22
    readonly property int entryPadding: 8
    readonly property int cornerRadius: 6

    readonly property int textPaddingX: 16
    readonly property int textPaddingY: 12
    readonly property int tabWidthInCharacters: 4
    readonly property int linesPerScrollStep: 1
    readonly property int linesKeptOnPageScroll: 2
    readonly property int noticeHeight: 28

    readonly property int pageSpacing: 16
    readonly property int pdfLineStep: 40
    readonly property real pdfMaximumZoom: 96 / 72

    readonly property int scrollBarWidth: 6
    readonly property int scrollBarMargin: 4
    readonly property int scrollBarMinimumLength: 24
    readonly property real scrollBarOpacity: 0.35
}
