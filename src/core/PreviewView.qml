import QtQuick

Item {
    property Controller controller: null
    property size available: Qt.size(0, 0)
    property real preferredWidth: 0
    property real preferredHeight: 0
    property bool ready: false
    property string error: ""
}
