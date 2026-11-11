import QtQuick

PreviewView {
    id: view

    ready: overview.finished
    error: view.controller ? view.controller.fileError : ""
    preferredWidth: overview.implicitWidth
    preferredHeight: overview.implicitHeight

    FileOverview {
        id: overview

        anchors.fill: parent
        source: view.controller && view.controller.fileError === "" ? view.controller.currentFileUrl : ""
        message: view.controller ? view.controller.notice : ""
        messageIcon: "dialog-information"
    }
}
