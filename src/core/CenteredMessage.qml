import QtQuick

Item {
    id: message

    property alias text: label.text

    Text {
        id: label

        anchors.centerIn: parent
        width: parent.width - 48
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        color: message.palette.windowText
        opacity: 0.7
        font.pixelSize: 16
    }
}
