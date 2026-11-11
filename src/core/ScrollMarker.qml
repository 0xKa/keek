import QtQuick

Rectangle {
    id: marker

    required property Flickable flickable

    anchors.right: parent.right
    anchors.rightMargin: Metrics.scrollBarMargin
    y: marker.flickable.y + marker.flickable.visibleArea.yPosition * marker.flickable.height
    width: Metrics.scrollBarWidth
    height: Math.max(Metrics.scrollBarMinimumLength, marker.flickable.visibleArea.heightRatio * marker.flickable.height)
    radius: Metrics.scrollBarWidth / 2
    color: marker.palette.text
    opacity: Metrics.scrollBarOpacity
    visible: marker.flickable.visibleArea.heightRatio < 1
}
