import QtQuick

Rectangle {
    id: root

    required property Controller controller
    readonly property PreviewView view: content.item as PreviewView
    readonly property bool contentReady: view ? view.ready : false
    readonly property string contentError: view ? view.error : ""
    property size availableSize: Qt.size(Metrics.popupDefaultWidth, Metrics.popupDefaultHeight)
    readonly property bool failed: contentError !== ""
    readonly property bool hasPreference: view !== null && !failed && view.preferredWidth > 0 && view.preferredHeight > 0
    readonly property bool sized: failed || hasPreference || contentReady
    readonly property real wantedWidth: failed ? errorOverview.implicitWidth : hasPreference ? view.preferredWidth : Metrics.popupDefaultWidth
    readonly property real wantedHeight: (failed ? errorOverview.implicitHeight : hasPreference ? view.preferredHeight : Metrics.popupDefaultHeight) + Metrics.titleHeight
    readonly property size preferredSize: Qt.size(Math.ceil(Math.min(availableSize.width, Math.max(Metrics.popupMinimumWidth, wantedWidth))), Math.ceil(Math.min(availableSize.height, Math.max(Metrics.popupMinimumHeight, wantedHeight))))

    color: palette.window
    focus: true

    Keys.forwardTo: view ? [view] : []
    Keys.onPressed: event => {
        if ((event.modifiers & ~Qt.KeypadModifier) !== Qt.NoModifier) {
            return;
        }
        switch (event.key) {
        case Qt.Key_Up:
        case Qt.Key_Left:
            root.controller.previous();
            break;
        case Qt.Key_Down:
        case Qt.Key_Right:
            root.controller.next();
            break;
        case Qt.Key_Space:
        case Qt.Key_Escape:
            if (!event.isAutoRepeat) {
                root.controller.hide();
            }
            break;
        case Qt.Key_Return:
        case Qt.Key_Enter:
            if (!event.isAutoRepeat) {
                root.controller.openCurrentFile();
            }
            break;
        default:
            return;
        }
        event.accepted = true;
    }

    Text {
        id: title

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: Metrics.margin
        anchors.rightMargin: Metrics.margin
        height: Metrics.titleHeight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideMiddle
        color: root.palette.windowText
        font.pixelSize: Metrics.titleFontSize
        text: root.controller.currentFileName
    }

    Loader {
        id: content

        anchors.top: title.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        active: root.controller.showing
        source: root.controller.rendererComponent + ".qml"
        opacity: root.failed ? 0 : 1
    }

    Binding {
        target: root.view
        property: "controller"
        value: root.controller
        when: root.view !== null
    }

    Binding {
        target: root.view
        property: "available"
        value: Qt.size(root.availableSize.width, root.availableSize.height - Metrics.titleHeight)
        when: root.view !== null
    }

    FileOverview {
        id: errorOverview

        anchors.fill: content
        source: root.failed ? root.controller.currentFileUrl : ""
        thumbnails: false
        message: root.contentError
        visible: root.failed
    }
}
