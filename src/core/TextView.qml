import QtQuick

PreviewView {
    id: view

    readonly property bool dark: palette.base.hslLightness < 0.5

    ready: file.ready
    error: file.error
    preferredWidth: Math.min(file.columns, Metrics.textColumns) * metrics.averageCharacterWidth + Metrics.textPaddingX + Metrics.margin
    preferredHeight: flickable.contentHeight + (notice.visible ? notice.height : 0)

    Keys.onPressed: event => {
        event.accepted = scroller.handle(event);
    }

    KeyScroller {
        id: scroller

        flickable: flickable
        lineStep: metrics.lineSpacing * Metrics.linesPerScrollStep
        pageOverlap: metrics.lineSpacing * Metrics.linesKeptOnPageScroll
    }

    TextFile {
        id: file

        source: view.controller ? view.controller.currentFileUrl : ""
        tabWidth: Metrics.tabWidthInCharacters
    }

    FontMetrics {
        id: metrics

        font: file.fixedFont
    }

    Rectangle {
        anchors.fill: parent
        color: view.palette.base
    }

    Flickable {
        id: flickable

        anchors.fill: parent
        anchors.bottomMargin: notice.visible ? notice.height : 0
        contentWidth: width
        contentHeight: text.height + Metrics.textPaddingY * 2
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        TextEdit {
            id: text

            x: Metrics.textPaddingX
            y: Metrics.textPaddingY
            width: Math.ceil(flickable.width - Metrics.textPaddingX - Metrics.margin)
            font: file.fixedFont
            color: view.palette.text
            textFormat: TextEdit.PlainText
            wrapMode: TextEdit.WrapAtWordBoundaryOrAnywhere
            tabStopDistance: metrics.averageCharacterWidth * Metrics.tabWidthInCharacters
            readOnly: true
            selectByMouse: false
            activeFocusOnPress: false
        }
    }

    ScrollMarker {
        flickable: flickable
    }

    Rectangle {
        id: notice

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: Metrics.noticeHeight
        color: view.palette.window
        visible: file.truncated

        Text {
            anchors.centerIn: parent
            color: view.palette.windowText
            text: qsTr("Only the first part of this file is shown")
        }
    }

    TextHighlighter {
        document: text.textDocument
        textFile: file
        dark: view.dark
    }
}
