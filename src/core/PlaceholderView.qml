import QtQuick

PreviewView {
    id: view

    ready: true

    Text {
        anchors.centerIn: parent
        width: parent.width - 48
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        color: view.palette.windowText
        opacity: 0.7
        font.pixelSize: 16
        text: view.controller ? view.controller.fileDescription : ""
    }
}
