pragma ComponentBehavior: Bound
import QtQuick 6.5
import Hydra.Backend 1.0
import "../styles"

Item {
    id: root

    property AppState appState: null
    property ThemeState themeState: null
    property DesktopDialogBridge desktopBridge: null
    property var helpHost: null
    property bool tightMode: false
    property real reveal: 1.0
    property real panelInset: 0
    property real expandedRailWidth: width
    property bool fullscreenActive: false

    readonly property bool focusWithin: sidebarContent ? sidebarContent.focusWithin : false
    readonly property var sectionSignalAnchors: sidebarContent ? sidebarContent.sectionSignalAnchors : []

    anchors.fill: parent

    function focusPrimaryControl() {
        sidebarContent.focusPrimaryControl()
    }

    RailAtmosphere {
        anchors.fill: parent
        appState: root.appState
        tightMode: root.tightMode
        reveal: root.reveal
        sectionAnchors: root.sectionSignalAnchors
    }

    Item {
        id: railContent

        width: Math.max(0, root.expandedRailWidth - (root.panelInset * 2))
        height: parent.height
        x: Math.round((root.reveal - 1.0) * HydraTheme.space16)
        opacity: Math.max(0.0, Math.min(1.0, (root.reveal - 0.08) / 0.92))

        LaunchSidebar {
            id: sidebarContent

            anchors.fill: parent
            appState: root.appState
            themeState: root.themeState
            desktopBridge: root.desktopBridge
            fullscreenActive: root.fullscreenActive
            layoutWidth: parent.width
            helpHost: root.helpHost
            onToggleFullscreenRequested: root.helpHost.toggleBorderlessFullscreen()
        }
    }
}
