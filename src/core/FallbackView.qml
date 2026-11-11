import QtQuick

PreviewView {
    id: view

    ready: !details.exists || summary.finished
    error: view.controller ? view.controller.fileError : ""
    preferredWidth: summary.implicitWidth + Metrics.margin * 2
    preferredHeight: summary.implicitHeight + Metrics.margin * 2

    FileDetails {
        id: details

        source: view.controller ? view.controller.currentFileUrl : ""
    }

    FileSummary {
        id: summary

        anchors.centerIn: parent
        source: details.exists ? details.source : ""
        iconName: details.iconName
        details: [details.typeName, details.size, qsTr("Modified %1").arg(details.modified)].filter(line => line !== "")
    }
}
