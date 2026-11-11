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
        if (event.isAutoRepeat) {
            return;
        }
        if (event.key === Qt.Key_Space || event.key === Qt.Key_Escape) {
            root.controller.hide();
            event.accepted = true;
        }
    }

    Text {
        id: title

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        height: 40
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideMiddle
        color: root.palette.windowText
        font.pixelSize: 14
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
