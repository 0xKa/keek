pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Pdf
import QtQuick.Window

PreviewView {
    id: view

    property bool firstPageShown: false
    property bool locked: false
    readonly property real pageWidth: Math.max(0, list.width - Metrics.margin * 2)
    readonly property size firstPage: document.status === PdfDocument.Ready && document.pageCount > 0 ? document.pagePointSize(0) : Qt.size(0, 0)
    readonly property real preferredZoom: firstPage.width <= 0 || firstPage.height <= 0 ? 0 : Math.min(Metrics.pdfMaximumZoom, (view.available.width - Metrics.margin * 2) / firstPage.width, (view.available.height - Metrics.pageSpacing * 2) / firstPage.height)

    ready: document.status === PdfDocument.Ready && (document.pageCount === 0 || view.firstPageShown)
    error: view.locked ? qsTr("This PDF is protected by a password.") : document.status === PdfDocument.Error ? qsTr("This PDF could not be opened. It may be damaged.") : ""
    preferredWidth: view.preferredZoom > 0 ? Math.round(view.firstPage.width * view.preferredZoom) + Metrics.margin * 2 : 0
    preferredHeight: view.preferredZoom > 0 ? Math.round(view.firstPage.height * view.preferredZoom) + Metrics.pageSpacing * 2 : 0

    Keys.onPressed: event => {
        event.accepted = scroller.handle(event);
    }

    KeyScroller {
        id: scroller

        flickable: list
        lineStep: Metrics.pdfLineStep
        pageOverlap: Metrics.pdfLineStep
    }

    PdfDocument {
        id: document

        source: view.controller ? view.controller.currentFileUrl : ""
        onSourceChanged: {
            view.firstPageShown = false;
            view.locked = false;
        }
        onPasswordRequired: view.locked = true
    }

    ListView {
        id: list

        anchors.fill: parent
        topMargin: Metrics.pageSpacing
        bottomMargin: Metrics.pageSpacing
        spacing: Metrics.pageSpacing
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        model: document.status === PdfDocument.Ready ? document.pageCount : 0

        delegate: Item {
            id: page

            required property int index
            readonly property size points: document.pagePointSize(page.index)
            readonly property real zoom: page.points.width > 0 ? Math.min(view.pageWidth / page.points.width, Metrics.pdfMaximumZoom) : 0

            width: list.width
            height: Math.round(page.points.height * page.zoom)

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.round(page.points.width * page.zoom)
                height: parent.height
                color: "white"

                PdfPageImage {
                    id: image

                    anchors.fill: parent
                    document: document
                    currentFrame: page.index
                    sourceSize.width: width * Screen.devicePixelRatio
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    cache: false

                    onStatusChanged: {
                        if (page.index === 0 && image.status === Image.Ready) {
                            view.firstPageShown = true;
                        }
                    }
                }
            }
        }
    }

    ScrollMarker {
        flickable: list
    }
}
