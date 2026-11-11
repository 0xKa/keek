import QtQuick
import QtMultimedia

MediaView {
    id: view

    property bool frameShown: false

    video: true
    stageReady: view.frameShown
    onSourceChanged: view.frameShown = false

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
        onTriggered: {
            if (view.controller) {
                view.controller.useFallback();
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: view.togglePlaying()
    }
}
