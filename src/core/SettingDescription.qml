import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Label {
    id: description

    property RadioButton button: null

    Layout.fillWidth: true
    Layout.maximumWidth: Metrics.messageWidth
    leftPadding: description.button ? description.button.indicator.width + description.button.spacing : 0
    wrapMode: Text.WordWrap
    opacity: Metrics.mutedOpacity
}
