pragma ComponentBehavior: Bound
import QtQuick 6.5
import Hydra.Backend 1.0
import "../styles"

Item {
    id: root

    property AppState appState: null
    property MasterState masterState: null
    property RouterState routerState: null
    property ShellState shellState: null
    property ThemeState themeState: null
    property DesktopDialogBridge desktopBridge: null
    property TerminalSurfaceController terminalController: null
    property TerminalSurfaceController masterTerminalController: null
    property TerminalSurfaceController routerTerminalController: null
    property var helpHost: null
    property bool startupShellReady: false
    property bool sidebarBackendReady: false
    property bool workbenchBackendReady: false
    property bool masterBackendReady: false
    property bool sidebarCollapsed: false
    property real sidebarReveal: 1.0
    property real panelInset: 0
    property real expandedRailWidth: 0
    property real railVisibleWidth: 0
    property real dividerCenterX: 0
    property real boardX: 0
    property int dividerWidth: 0
    property real frameRadius: 0
    property bool tightShell: false
    property bool fullscreenActive: false
    property bool masterViewLoaded: false
    property string activeViewMode: "workbench"
    property string startupExpandedSessionName: ""

    readonly property var sidebarItem: sidebarLoader.item ? sidebarLoader.item : null
    readonly property var workbenchItem: workbenchLoader.item ? workbenchLoader.item : null
    readonly property var masterItem: masterTerminalLoader.item ? masterTerminalLoader.item : null
    readonly property bool dividerDragActive: dividerHandle.dragActive

    signal toggleSidebarRequested()
    signal dividerDragStarted()
    signal dividerDragMoved(real deltaX)
    signal dividerDragFinished()
    signal activateWorkbenchRequested()
    signal activateMasterRequested()

    anchors.fill: parent
    visible: root.startupShellReady
    enabled: root.startupShellReady

    Component {
        id: workbenchPaneComponent

        WorkbenchPane {
            visible: root.activeViewMode === "workbench"
            enabled: visible
            appState: root.appState
            shellState: root.shellState
            terminalController: root.terminalController
            helpHost: root.helpHost
            startupExpandedSessionName: root.startupExpandedSessionName
        }
    }

    Component {
        id: masterTerminalPaneComponent

        MasterTerminalPane {
            visible: root.activeViewMode === "master"
            enabled: visible
            appState: root.appState
            masterState: root.masterState
            routerState: root.routerState
            shellState: root.shellState
            terminalController: root.masterTerminalController
            routerTerminalController: root.routerTerminalController
            helpHost: root.helpHost
        }
    }

    Item {
        id: railFrame

        x: 0
        y: 0
        width: root.railVisibleWidth
        height: parent.height
        clip: true
        z: 1

        Rectangle {
            anchors.fill: parent
            radius: root.frameRadius
            color: HydraTheme.railBg
            border.width: 1
            border.color: HydraTheme.borderDark
            clip: true
            opacity: root.sidebarReveal

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: root.tightShell ? 18 : 22
                color: HydraTheme.withAlpha(HydraTheme.boardPanelMuted,
                                            HydraTheme.currentThemeId === "eva" ? 0.12 : 0.44)
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: root.tightShell ? 18 : 22
                height: 1
                color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.24)
            }

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 2
                color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.7)
            }

            FrameCorners {
                anchors.fill: parent
                lineColor: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.24)
            }

            Item {
                id: railViewport

                anchors.fill: parent
                anchors.margins: root.panelInset
                clip: true

                Loader {
                    id: sidebarLoader
                    anchors.fill: parent
                    active: root.sidebarBackendReady
                    sourceComponent: root.sidebarBackendReady ? railSidebarComponent : undefined
                }
            }

            Rectangle {
                anchors.fill: parent
                radius: root.frameRadius
                color: "transparent"
                visible: Boolean(root.sidebarItem && root.sidebarItem.focusWithin)
                border.width: 1
                border.color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.34)
            }
        }
    }

    Component {
        id: railSidebarComponent

        RailSidebarShell {
            appState: root.appState
            themeState: root.themeState
            desktopBridge: root.desktopBridge
            helpHost: root.helpHost
            tightMode: root.tightShell
            reveal: root.sidebarReveal
            panelInset: root.panelInset
            expandedRailWidth: root.expandedRailWidth
            fullscreenActive: root.fullscreenActive
        }
    }

    Item {
        id: dividerTrack

        x: root.dividerCenterX - (width * 0.5)
        y: 0
        width: root.dividerWidth
        height: parent.height
        z: 3

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: HydraTheme.withAlpha(HydraTheme.borderDark, 0.34)
        }

        DividerHandle {
            id: dividerHandle

            tightMode: root.tightShell
            sidebarCollapsed: root.sidebarCollapsed
            hoverHost: root.helpHost
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: root.panelInset + HydraTheme.space8
            onToggleRequested: root.toggleSidebarRequested()
            onDragStarted: root.dividerDragStarted()
            onDragMoved: deltaX => root.dividerDragMoved(deltaX)
            onDragFinished: root.dividerDragFinished()
        }
    }

    Rectangle {
        id: boardFrame

        x: root.boardX
        y: 0
        width: parent.width - x
        height: parent.height
        radius: root.frameRadius
        color: HydraTheme.boardBg
        border.width: 1
        border.color: HydraTheme.borderDark
        clip: true
        z: 1

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: root.tightShell ? 18 : 22
            color: HydraTheme.withAlpha(HydraTheme.boardPanelMuted, 0.42)
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: root.tightShell ? 18 : 22
            height: 1
            color: HydraTheme.withAlpha(HydraTheme.accentSteel, 0.22)
        }

        FrameCorners {
            anchors.fill: parent
            lineColor: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.22)
        }

        Row {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: 2
            anchors.rightMargin: HydraTheme.space16
            z: 2
            spacing: HydraTheme.space4

            ActionChip {
                text: "[WORKBENCH]"
                toneColor: root.activeViewMode === "workbench"
                           ? HydraTheme.accentBronze
                           : HydraTheme.accentSteelBright
                active: root.activeViewMode === "workbench"
                accessibleLabel: "Switch to Workbench view"
                onClicked: root.activateWorkbenchRequested()
            }

            ActionChip {
                text: "[MASTER]"
                toneColor: root.activeViewMode === "master"
                           ? HydraTheme.accentSignal
                           : HydraTheme.accentSteelBright
                active: root.activeViewMode === "master"
                accessibleLabel: "Switch to Master Terminal view"
                onClicked: root.activateMasterRequested()
            }
        }

        Loader {
            id: workbenchLoader
            anchors.fill: parent
            anchors.margins: root.panelInset
            active: root.startupShellReady && root.workbenchBackendReady
            visible: active && root.activeViewMode === "workbench"
            sourceComponent: root.workbenchBackendReady ? workbenchPaneComponent : undefined
        }

        Loader {
            id: masterTerminalLoader
            anchors.fill: parent
            anchors.margins: root.panelInset
            active: root.startupShellReady && root.masterViewLoaded && root.masterBackendReady
            visible: active && root.activeViewMode === "master"
            sourceComponent: root.masterBackendReady ? masterTerminalPaneComponent : undefined
        }
    }
}
