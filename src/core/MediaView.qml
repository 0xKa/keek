pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtMultimedia

PreviewView {
    id: view

    property bool video: true
    property bool stageReady: true
    readonly property bool muted: view.video ? MediaSettings.videoMuted : MediaSettings.audioMuted
    readonly property bool loaded: player.mediaStatus !== MediaPlayer.NoMedia && player.mediaStatus !== MediaPlayer.LoadingMedia && player.mediaStatus !== MediaPlayer.InvalidMedia
    readonly property bool failed: player.error !== MediaPlayer.NoError || player.mediaStatus === MediaPlayer.InvalidMedia || (view.loaded && (view.video ? !player.hasVideo : !player.hasAudio))
    readonly property url source: view.controller ? view.controller.currentFileUrl : ""
    default property alias content: stage.data
    readonly property MediaPlayer player: player
    readonly property string title: player.metaData.stringValue(MediaMetaData.Title)
    readonly property string artist: player.metaData.stringValue(MediaMetaData.ContributingArtist) || player.metaData.stringValue(MediaMetaData.AlbumArtist)
    readonly property string album: player.metaData.stringValue(MediaMetaData.AlbumTitle)

    function formatTime(ms: real): string {
        const total = Math.max(0, Math.floor(ms / 1000));
        const hours = Math.floor(total / 3600);
        const minutes = Math.floor(total / 60) % 60;
        const seconds = String(total % 60).padStart(2, "0");
        return hours > 0 ? `${hours}:${String(minutes).padStart(2, "0")}:${seconds}` : `${minutes}:${seconds}`;
    }

    function togglePlaying() {
        if (player.playbackState === MediaPlayer.PlayingState) {
            player.pause();
        } else {
            player.play();
        }
    }

    function toggleMuted() {
        if (view.video) {
            MediaSettings.videoMuted = !MediaSettings.videoMuted;
        } else {
            MediaSettings.audioMuted = !MediaSettings.audioMuted;
        }
    }

    function start() {
        if (player.source.toString() === "") {
            return;
        }
        if (MediaSettings.autoplay) {
            player.play();
        } else {
            player.pause();
        }
    }

    function seekBy(ms: real) {
        player.position = Math.max(0, Math.min(player.duration, player.position + ms));
    }

    function giveUp() {
        if (view.controller) {
            view.controller.useFallback(view.video ? qsTr("This video cannot be played. It may need a codec that is not installed.") : qsTr("This audio file cannot be played. It may need a codec that is not installed."));
        }
    }

    ready: view.loaded && !view.failed && view.stageReady
    onFailedChanged: {
        if (view.failed) {
            view.giveUp();
        }
    }

    Keys.onPressed: event => {
        const shift = event.modifiers === Qt.ShiftModifier;
        const plain = (event.modifiers & ~Qt.KeypadModifier) === Qt.NoModifier;
        if (plain && event.key === Qt.Key_K) {
            if (!event.isAutoRepeat) {
                view.togglePlaying();
            }
        } else if (plain && event.key === Qt.Key_M) {
            if (!event.isAutoRepeat) {
                view.toggleMuted();
            }
        } else if (shift && event.key === Qt.Key_Left) {
            view.seekBy(-Metrics.seekStepMs);
        } else if (shift && event.key === Qt.Key_Right) {
            view.seekBy(Metrics.seekStepMs);
        } else {
            return;
        }
        event.accepted = true;
    }

    MediaPlayer {
        id: player

        objectName: "player"
        source: view.source
        audioOutput: AudioOutput {
            muted: view.muted
        }
        onSourceChanged: view.start()
        Component.onCompleted: view.start()
    }

    Item {
        id: stage

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: controls.top
    }

    Rectangle {
        id: controls

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: Metrics.controlBarHeight
        color: view.palette.window

        Row {
            id: controlRow

            anchors.fill: parent
            anchors.leftMargin: Metrics.entryPadding
            anchors.rightMargin: Metrics.entryPadding
            spacing: Metrics.entryPadding

            ToolButton {
                id: playButton

                anchors.verticalCenter: parent.verticalCenter
                focusPolicy: Qt.NoFocus
                icon.color: view.palette.windowText
                icon.name: player.playbackState === MediaPlayer.PlayingState ? "media-playback-pause" : "media-playback-start"
                onClicked: view.togglePlaying()
            }

            Label {
                id: elapsed

                anchors.verticalCenter: parent.verticalCenter
                width: Metrics.timeLabelWidth
                horizontalAlignment: Text.AlignRight
                color: view.palette.windowText
                text: view.formatTime(player.position)
            }

            Slider {
                anchors.verticalCenter: parent.verticalCenter
                width: controlRow.width - playButton.width - elapsed.width - remaining.width - muteButton.width - controlRow.spacing * 4
                focusPolicy: Qt.NoFocus
                from: 0
                to: Math.max(1, player.duration)
                value: player.position
                enabled: player.seekable
                onMoved: player.position = value
            }

            Label {
                id: remaining

                anchors.verticalCenter: parent.verticalCenter
                width: Metrics.timeLabelWidth
                color: view.palette.windowText
                text: view.formatTime(player.duration)
            }

            ToolButton {
                id: muteButton

                anchors.verticalCenter: parent.verticalCenter
                focusPolicy: Qt.NoFocus
                icon.color: view.palette.windowText
                icon.name: view.muted ? "audio-volume-muted" : "audio-volume-high"
                onClicked: view.toggleMuted()
            }
        }
    }
}
