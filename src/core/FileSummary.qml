pragma ComponentBehavior: Bound

import QtQuick

Column {
    id: summary

    property url source
    property string iconName
    property list<string> details
    property bool thumbnails: true
    readonly property bool finished: thumbnail.finished

    spacing: Metrics.detailSpacing

    Item {
        anchors.horizontalCenter: parent.horizontalCenter
        width: Metrics.thumbnailSize
        height: thumbnail.available ? Math.min(Metrics.thumbnailSize, thumbnail.implicitHeight) : Metrics.iconSize

        Thumbnail {
            id: thumbnail

            anchors.fill: parent
            source: summary.thumbnails ? summary.source : ""
            maximumSize: Metrics.thumbnailSize
            visible: thumbnail.available
        }

        FileIcon {
            anchors.centerIn: parent
            width: Metrics.iconSize
            height: Metrics.iconSize
            name: summary.iconName
            visible: !thumbnail.available
        }
    }

    Repeater {
        model: summary.details

        Text {
            required property string modelData
            required property int index

            anchors.horizontalCenter: parent.horizontalCenter
            color: summary.palette.windowText
            opacity: index === 0 ? 1 : Metrics.mutedOpacity
            font.pixelSize: index === 0 ? Metrics.messageFontSize : Metrics.detailFontSize
            text: modelData
        }
    }
}
