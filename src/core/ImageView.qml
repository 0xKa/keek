import QtQuick
import QtQuick.Window

PreviewView {
    id: view

    readonly property bool sized: info.size.width > 0 && info.size.height > 0
    readonly property real fit: sized ? Math.min(view.width / info.size.width, view.height / info.size.height) : 1
    readonly property real zoom: info.scalable ? fit : Math.min(fit, 1 / Screen.devicePixelRatio)

    ready: image.status === Image.Ready
    error: image.status === Image.Error ? qsTr("This image could not be decoded") : ""

    ImageInfo {
        id: info

        source: view.controller ? view.controller.currentFileUrl : ""
    }

    Image {
        id: image

        anchors.centerIn: parent
        width: view.sized ? Math.round(info.size.width * view.zoom) : view.width
        height: view.sized ? Math.round(info.size.height * view.zoom) : view.height
        source: view.width > 0 && view.height > 0 ? info.source : ""
        sourceSize: Qt.size(width, height)
        fillMode: Image.PreserveAspectFit
        asynchronous: true
        autoTransform: true
        cache: false
    }
}
