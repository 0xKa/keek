import QtQuick
import QtMultimedia

MediaView {
    id: view

    property bool frameShown: false
    readonly property size resolution: {
        const tagged = view.player.metaData.value(MediaMetaData.Resolution);
        if (tagged && tagged.width > 0 && tagged.height > 0) {
            return tagged;
        }
        return Qt.size(output.sourceRect.width, output.sourceRect.height);
    }
    readonly property real stageHeight: view.available.height - Metrics.controlBarHeight
    readonly property real preferredZoom: view.resolution.width <= 0 || view.resolution.height <= 0 || view.stageHeight <= 0 ? 0 : Math.min(1, view.available.width / view.resolution.width, view.stageHeight / view.resolution.height)

    video: true
    stageReady: view.frameShown
    onSourceChanged: view.frameShown = false
    preferredWidth: Math.round(view.resolution.width * view.preferredZoom)
    preferredHeight: view.preferredZoom > 0 ? Math.round(view.resolution.height * view.preferredZoom) + Metrics.controlBarHeight : 0

    VideoOutput {
        id: output

        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectFit
        Component.onCompleted: view.player.videoOutput = output
    }

    Connections {
        target: output.videoSink

        function onVideoFrameChanged() {
            view.frameShown = true;
        }
    }

    Timer {
        interval: Metrics.firstFrameTimeoutMs
        running: view.loaded && !view.failed && !view.frameShown
        onTriggered: view.giveUp()
    }

    MouseArea {
        anchors.fill: parent
        onClicked: view.togglePlaying()
    }
}
