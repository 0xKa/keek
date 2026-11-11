import QtQuick

MediaView {
    id: view

    video: false
    preferredWidth: summary.implicitWidth + Metrics.margin * 2
    preferredHeight: summary.implicitHeight + Metrics.margin * 2 + Metrics.controlBarHeight

    FileDetails {
        id: details

        source: view.source
    }

    FileSummary {
        id: summary

        anchors.centerIn: parent
        source: details.exists ? details.source : ""
        iconName: details.iconName
        details: [view.title || details.typeName, view.artist, view.album].filter(line => line !== "")
    }
}
