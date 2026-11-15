import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page

    implicitWidth: column.implicitWidth + Metrics.margin * 2
    implicitHeight: column.implicitHeight + Metrics.margin * 2

    ButtonGroup {
        id: whenClosedGroup
    }

    ColumnLayout {
        id: column

        anchors.fill: parent
        anchors.margins: Metrics.margin
        spacing: Metrics.detailSpacing

        Label {
            text: qsTr("When the popup closes")
            font.bold: true
        }

        RadioButton {
            id: keepReady

            objectName: "keepReady"
            text: qsTr("Keep Keek ready")
            ButtonGroup.group: whenClosedGroup
            onClicked: GeneralSettings.whenClosed = GeneralSettings.WhenClosed.KeepReady

            Binding on checked {
                value: GeneralSettings.whenClosed === GeneralSettings.WhenClosed.KeepReady
            }
        }

        SettingDescription {
            button: keepReady
            text: qsTr("The next preview opens instantly. Keek keeps what it has loaded in memory.")
        }

        RadioButton {
            id: freeMemory

            objectName: "freeMemory"
            text: qsTr("Free memory")
            ButtonGroup.group: whenClosedGroup
            onClicked: GeneralSettings.whenClosed = GeneralSettings.WhenClosed.FreeMemory

            Binding on checked {
                value: GeneralSettings.whenClosed === GeneralSettings.WhenClosed.FreeMemory
            }
        }

        SettingDescription {
            button: freeMemory
            text: qsTr("Keek unloads the preview a few seconds after the popup closes. The next preview takes a moment longer to open.")
        }

        RowLayout {
            spacing: Metrics.detailSpacing

            RadioButton {
                id: stop

                objectName: "stop"
                text: qsTr("Stop Keek after")
                ButtonGroup.group: whenClosedGroup
                onClicked: GeneralSettings.whenClosed = GeneralSettings.WhenClosed.Stop

                Binding on checked {
                    value: GeneralSettings.whenClosed === GeneralSettings.WhenClosed.Stop
                }
            }

            SpinBox {
                id: count

                objectName: "stopAfter"
                from: 0
                to: GeneralSettings.maximumStopAfter
                editable: true
                enabled: stop.checked
                onValueModified: GeneralSettings.stopAfter = count.value

                Binding on value {
                    value: GeneralSettings.stopAfter
                }
            }

            ComboBox {
                id: unit

                objectName: "stopAfterUnit"
                model: [qsTr("seconds", "", count.value), qsTr("minutes", "", count.value)]
                enabled: stop.checked
                onActivated: index => GeneralSettings.stopAfterUnit = index === 0 ? GeneralSettings.Unit.Seconds : GeneralSettings.Unit.Minutes

                Binding on currentIndex {
                    value: GeneralSettings.stopAfterUnit === GeneralSettings.Unit.Seconds ? 0 : 1
                }
            }
        }

        SettingDescription {
            button: stop
            text: qsTr("Keek uses no memory once it has stopped. The next preview takes as long as the first one after login. With 0, Keek stops as soon as the popup closes.")
        }

        Label {
            Layout.topMargin: Metrics.sectionSpacing
            text: qsTr("Audio and video")
            font.bold: true
        }

        CheckBox {
            text: qsTr("Mute audio")
            onToggled: MediaSettings.audioMuted = checked

            Binding on checked {
                value: MediaSettings.audioMuted
            }
        }

        CheckBox {
            text: qsTr("Mute video")
            onToggled: MediaSettings.videoMuted = checked

            Binding on checked {
                value: MediaSettings.videoMuted
            }
        }

        SettingDescription {
            text: qsTr("Pressing M in the popup changes these too.")
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
