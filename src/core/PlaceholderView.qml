import QtQuick

PreviewView {
    id: view

    ready: true

    CenteredMessage {
        anchors.fill: parent
        text: view.controller ? view.controller.fileDescription : ""
    }
}
