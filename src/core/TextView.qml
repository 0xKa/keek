import QtQuick
import org.kde.syntaxhighlighting

PreviewView {
    id: view

    readonly property bool dark: palette.base.hslLightness < 0.5

    ready: file.ready
    error: file.error

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
            width: flickable.width - Metrics.textPaddingX - Metrics.margin
            text: file.text
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

    SyntaxHighlighter {
        textEdit: text
        definition: Repository.definitionForFileName(file.fileName)
        theme: Repository.defaultTheme(view.dark ? Repository.DarkTheme : Repository.LightTheme)
    }
}
