import QtQuick

Item {
    id: message

    property alias text: label.text

    Text {
        id: label

        anchors.centerIn: parent
        width: parent.width - Metrics.margin * 2
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        color: message.palette.windowText
        opacity: Metrics.mutedOpacity
        font.pixelSize: Metrics.messageFontSize
    }
}
