import QtQuick

Rectangle {
    id: root

    required property Controller controller

    color: palette.window
    focus: true

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
        anchors.centerIn: parent
        width: parent.width - 48
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideMiddle
        color: root.palette.windowText
        font.pixelSize: 20
        text: root.controller.currentFileName
    }
}
