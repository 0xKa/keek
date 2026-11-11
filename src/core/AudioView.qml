import QtQuick

MediaView {
    id: view

    video: false

    FileDetails {
        id: details

        source: view.source
    }

    FileSummary {
        anchors.centerIn: parent
        source: details.exists ? details.source : ""
        iconName: details.iconName
        details: [view.title || details.typeName, view.artist, view.album].filter(line => line !== "")
    }
}
