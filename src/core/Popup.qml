import QtQuick

Rectangle {
    id: root

    required property Controller controller
    readonly property PreviewView view: content.item as PreviewView
    readonly property bool contentReady: view ? view.ready : false
    readonly property string contentError: view ? view.error : ""

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
        opacity: root.contentError === "" ? 1 : 0
    }

    Binding {
        target: root.view
        property: "controller"
        value: root.controller
        when: root.view !== null
    }

    CenteredMessage {
        anchors.fill: content
        text: root.contentError
        visible: root.contentError !== ""
    }
}
