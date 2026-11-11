pragma Singleton
import QtQuick

QtObject {
    readonly property int popupMinimumWidth: 480
    readonly property int popupMinimumHeight: 320
    readonly property int popupDefaultWidth: 960
    readonly property int popupDefaultHeight: 640
    readonly property int textColumns: 100
    readonly property int vectorImageSize: 512
    readonly property int revealDuration: 120

    readonly property int margin: 24
    readonly property int titleHeight: 40
    readonly property int titleFontSize: 14
    readonly property int messageFontSize: 16
    readonly property int messageIconSize: 22
    readonly property int messageWidth: 420
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

    readonly property int controlBarHeight: 48
    readonly property int timeLabelWidth: 56
    readonly property int seekStepMs: 5000
    readonly property int firstFrameTimeoutMs: 3000

    readonly property int pageSpacing: 16
    readonly property int pdfLineStep: 40
    readonly property real pdfMaximumZoom: 96 / 72

    readonly property int scrollBarWidth: 6
    readonly property int scrollBarMargin: 4
    readonly property int scrollBarMinimumLength: 24
    readonly property real scrollBarOpacity: 0.35
}
