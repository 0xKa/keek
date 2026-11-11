import QtQuick

Rectangle {
    id: root

    required property Controller controller
    readonly property PreviewView view: content.item as PreviewView
    readonly property bool contentReady: view ? view.ready : false
    readonly property string contentError: view ? view.error : ""
    property size availableSize: Qt.size(Metrics.popupDefaultWidth, Metrics.popupDefaultHeight)
    readonly property bool hasPreference: view !== null && contentError === "" && view.preferredWidth > 0 && view.preferredHeight > 0
    readonly property bool sized: contentError !== "" || hasPreference || contentReady
    readonly property real wantedWidth: hasPreference ? view.preferredWidth : contentError === "" ? Metrics.popupDefaultWidth : 0
    readonly property real wantedHeight: (hasPreference ? view.preferredHeight : contentError === "" ? Metrics.popupDefaultHeight : 0) + Metrics.titleHeight
    readonly property size preferredSize: Qt.size(Math.ceil(Math.min(availableSize.width, Math.max(Metrics.popupMinimumWidth, wantedWidth))), Math.ceil(Math.min(availableSize.height, Math.max(Metrics.popupMinimumHeight, wantedHeight))))

    color: palette.window
    focus: true

    Keys.forwardTo: view ? [view] : []
    Keys.onPressed: event => {
        if ((event.modifiers & ~Qt.KeypadModifier) !== Qt.NoModifier) {
            return;
        }
        switch (event.key) {
        case Qt.Key_Up:
        case Qt.Key_Left:
            root.controller.previous();
            break;
        case Qt.Key_Down:
        case Qt.Key_Right:
            root.controller.next();
            break;
        case Qt.Key_Space:
        case Qt.Key_Escape:
            if (!event.isAutoRepeat) {
                root.controller.hide();
            }
            break;
        case Qt.Key_Return:
        case Qt.Key_Enter:
            if (!event.isAutoRepeat) {
                root.controller.openCurrentFile();
            }
            break;
        default:
            return;
        }
        event.accepted = true;
    }

    Text {
        id: title

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: Metrics.margin
        anchors.rightMargin: Metrics.margin
        height: Metrics.titleHeight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideMiddle
        color: root.palette.windowText
        font.pixelSize: Metrics.titleFontSize
        text: root.controller.currentFileName
    }

    Loader {
        id: content

        anchors.top: title.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        active: root.controller.showing
        source: root.controller.rendererComponent + ".qml"
        opacity: root.contentError === "" ? 1 : 0
    }

    Binding {
        target: root.view
        property: "controller"
        value: root.controller
        when: root.view !== null
    }

    Binding {
        target: root.view
        property: "available"
        value: Qt.size(root.availableSize.width, root.availableSize.height - Metrics.titleHeight)
        when: root.view !== null
    }

    CenteredMessage {
        anchors.fill: content
        text: root.contentError
        visible: root.contentError !== ""
    }
}
