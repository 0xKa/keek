import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root

    header: TabBar {
        id: tabs

        TabButton {
            width: implicitWidth
            text: qsTr("Settings")
        }
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: tabs.currentIndex

        SettingsPage {}
    }

    Shortcut {
        sequences: [StandardKey.Close]
        onActivated: root.Window.window.close()
    }
}
