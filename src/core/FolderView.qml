pragma ComponentBehavior: Bound

import QtQuick

PreviewView {
    id: view

    readonly property int hiddenEntries: info.fileCount + info.folderCount - info.entries.length

    function countText(): string {
        const files = info.fileCount === 1 ? qsTr("1 file") : qsTr("%1 files").arg(info.fileCount);
        const folders = info.folderCount === 1 ? qsTr("1 folder") : qsTr("%1 folders").arg(info.folderCount);
        if (info.folderCount === 0) {
            return info.fileCount === 0 ? qsTr("Empty folder") : files;
        }
        return info.fileCount === 0 ? folders : qsTr("%1, %2").arg(files).arg(folders);
    }

    ready: !info.counting

    FolderInfo {
        id: info

        source: view.controller ? view.controller.currentFileUrl : ""
        maximumEntries: Metrics.folderEntries
    }

    FileDetails {
        id: details

        source: view.controller ? view.controller.currentFileUrl : ""
    }

    Column {
        anchors.centerIn: parent
        spacing: Metrics.sectionSpacing

        FileSummary {
            anchors.horizontalCenter: parent.horizontalCenter
            thumbnails: false
            iconName: details.iconName
            details: [view.countText(), info.counting ? qsTr("Counting… %1").arg(info.totalSize) : info.totalSize, qsTr("Modified %1").arg(details.modified)]
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: Metrics.entryListWidth
            height: entryColumn.height + Metrics.entryPadding * 2
            radius: Metrics.cornerRadius
            color: view.palette.base
            visible: info.entries.length > 0

            Column {
                id: entryColumn

                x: Metrics.entryPadding
                y: Metrics.entryPadding
                width: parent.width - Metrics.entryPadding * 2

                Repeater {
                    model: info.entries

                    Row {
                        id: entry

                        required property var modelData

                        height: Metrics.entryHeight
                        spacing: Metrics.entryPadding

                        FileIcon {
                            anchors.verticalCenter: parent.verticalCenter
                            width: Metrics.entryIconSize
                            height: Metrics.entryIconSize
                            name: entry.modelData.iconName
                        }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            width: entryColumn.width - Metrics.entryIconSize - Metrics.entryPadding
                            elide: Text.ElideMiddle
                            color: view.palette.text
                            font.pixelSize: Metrics.detailFontSize
                            text: entry.modelData.name
                        }
                    }
                }

                Text {
                    height: Metrics.entryHeight
                    verticalAlignment: Text.AlignVCenter
                    color: view.palette.text
                    opacity: Metrics.mutedOpacity
                    font.pixelSize: Metrics.detailFontSize
                    text: qsTr("and %1 more").arg(view.hiddenEntries)
                    visible: view.hiddenEntries > 0
                }
            }
        }
    }
}
