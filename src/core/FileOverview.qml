import QtQuick

Item {
    id: overview

    property url source
    property bool thumbnails: true
    property alias message: summary.message
    property alias messageIcon: summary.messageIcon
    readonly property bool exists: details.exists
    readonly property bool finished: !details.exists || !overview.thumbnails || summary.finished

    implicitWidth: summary.implicitWidth + Metrics.margin * 2
    implicitHeight: summary.implicitHeight + Metrics.margin * 2

    FileDetails {
        id: details

        source: overview.source
    }

    FileSummary {
        id: summary

        anchors.centerIn: parent
        source: details.exists ? details.source : ""
        thumbnails: overview.thumbnails
        iconName: details.iconName
        details: [details.typeName, details.size, details.modified === "" ? "" : qsTr("Modified %1").arg(details.modified)].filter(line => line !== "")
    }
}
