import QtQuick
import QtQuick.Window

PreviewView {
    id: view

    readonly property bool sized: info.size.width > 0 && info.size.height > 0
    readonly property real fit: sized ? Math.min(view.width / info.size.width, view.height / info.size.height) : 1
    readonly property real zoom: info.scalable ? fit : Math.min(fit, 1 / Screen.devicePixelRatio)
    readonly property real naturalZoom: !sized ? 0 : info.scalable ? Metrics.vectorImageSize / Math.max(info.size.width, info.size.height) : 1 / Screen.devicePixelRatio
    readonly property real preferredZoom: !sized || view.available.width <= 0 || view.available.height <= 0 ? 0 : Math.min(naturalZoom, view.available.width / info.size.width, view.available.height / info.size.height)

    preferredWidth: Math.round(info.size.width * view.preferredZoom)
    preferredHeight: Math.round(info.size.height * view.preferredZoom)

    property bool failed: false

    ready: image.status === Image.Ready
    error: view.failed ? qsTr("This image could not be decoded") : ""

    ImageInfo {
        id: info

        source: view.controller ? view.controller.currentFileUrl : ""
        onSourceChanged: view.failed = false
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
        onStatusChanged: {
            if (image.status === Image.Error) {
                view.failed = true;
            }
        }
    }
}
