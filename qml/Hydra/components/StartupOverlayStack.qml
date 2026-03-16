pragma ComponentBehavior: Bound
import QtQuick 6.5

Item {
    id: root

    property bool startupOverlayActive: false
    property bool startupSkipBoot: false
    property bool startupShellReady: false
    property bool bootScreenActive: false
    property bool nousSplashActive: false

    readonly property alias bootScreenItem: bootScreen
    readonly property alias nousSplashItem: nousSplash

    signal skipRequested()
    signal bootDismissed()
    signal splashDismissed()

    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        z: 25
        color: "#000000"
        visible: root.startupOverlayActive

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
        }
    }

    FocusScope {
        anchors.fill: parent
        z: 45
        visible: root.startupOverlayActive
        enabled: visible
        focus: visible

        Keys.onPressed: function(event) {
            if (!visible) {
                return
            }
            event.accepted = true
            if (event.key === Qt.Key_Escape) {
                root.skipRequested()
            }
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
            hoverEnabled: true
        }
    }

    BootScreen {
        id: bootScreen
        anchors.fill: parent
        z: 30
        active: root.bootScreenActive && !root.startupSkipBoot
        onDismissed: root.bootDismissed()
    }

    NousSplashScreen {
        id: nousSplash
        anchors.fill: parent
        z: 40
        active: root.nousSplashActive && !root.startupSkipBoot
        onDismissed: {
            if (root.startupShellReady) {
                return
            }
            root.splashDismissed()
        }
    }
}
