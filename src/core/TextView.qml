import QtQuick
import org.kde.syntaxhighlighting

PreviewView {
    id: view

    readonly property bool dark: palette.base.hslLightness < 0.5

    function scrollBy(distance: real) {
        const limit = Math.max(0, flickable.contentHeight - flickable.height);
        flickable.contentY = Math.max(0, Math.min(limit, flickable.contentY + distance));
    }

    ready: file.ready
    error: file.error

    Keys.onPressed: event => {
        const line = metrics.lineSpacing * Metrics.linesPerScrollStep;
        const page = flickable.height - metrics.lineSpacing * Metrics.linesKeptOnPageScroll;
        switch (event.key) {
        case Qt.Key_Down:
            view.scrollBy(line);
            break;
        case Qt.Key_Up:
            view.scrollBy(-line);
            break;
        case Qt.Key_PageDown:
            view.scrollBy(page);
            break;
        case Qt.Key_PageUp:
            view.scrollBy(-page);
            break;
        case Qt.Key_Home:
            view.scrollBy(-flickable.contentHeight);
            break;
        case Qt.Key_End:
            view.scrollBy(flickable.contentHeight);
            break;
        default:
            return;
        }
        event.accepted = true;
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

    Rectangle {
        anchors.right: parent.right
        anchors.rightMargin: Metrics.scrollBarMargin
        y: flickable.visibleArea.yPosition * flickable.height
        width: Metrics.scrollBarWidth
        height: Math.max(Metrics.scrollBarMinimumLength, flickable.visibleArea.heightRatio * flickable.height)
        radius: Metrics.scrollBarWidth / 2
        color: view.palette.text
        opacity: Metrics.scrollBarOpacity
        visible: flickable.visibleArea.heightRatio < 1
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
