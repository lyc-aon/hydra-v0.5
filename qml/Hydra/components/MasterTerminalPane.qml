pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import Hydra.Backend 1.0
import "../styles"

FocusScope {
    id: root

    required property AppState appState
    required property MasterState masterState
    required property RouterState routerState
    required property ShellState shellState
    required property TerminalSurfaceController terminalController
    required property TerminalSurfaceController routerTerminalController
    property var helpHost: null

    readonly property bool orbitStripFocused: Boolean(orbitStrip && orbitStrip.focusWithin)
    readonly property bool terminalSurfaceFocused: Boolean(masterSurface && masterSurface.focusWithin)
    readonly property bool routerPanelFocused: Boolean(routerPanel && routerPanel.focusWithin)

    activeFocusOnTab: true
    Accessible.role: Accessible.Grouping
    Accessible.name: "Master Terminal view"

    function focusMasterTerminal() {
        if (routerPanel && routerPanel.releaseTerminalFocus) {
            routerPanel.releaseTerminalFocus()
        }
        masterSurface.focusTerminalPanel()
    }

    function focusOrbitStrip() {
        orbitStrip.forceActiveFocus()
    }

    function focusRouterPane() {
        if (masterSurface && masterSurface.releaseTerminalFocus) {
            masterSurface.releaseTerminalFocus()
        }
        routerPanel.focusPrimaryControl()
    }

    MasterConfigDialog {
        id: masterConfigDialog
        appState: root.appState
        masterState: root.masterState
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: HydraTheme.space6

        SessionOrbitStrip {
            id: orbitStrip
            Layout.fillWidth: true
            Layout.preferredHeight: root.shellState.masterOrbitCollapsed ? implicitHeight : 124
            appState: root.appState
            collapsed: root.shellState.masterOrbitCollapsed
            stateDistribution: root.appState.sessionStateDistribution
            onCollapseToggled: root.shellState.masterOrbitCollapsed = !root.shellState.masterOrbitCollapsed
            onSessionClicked: sessionId => {
                root.appState.selectedSessionId = sessionId
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: HydraTheme.space8

            MasterTerminalSurface {
                id: masterSurface
                Layout.fillWidth: true
                Layout.fillHeight: true
                appState: root.appState
                masterState: root.masterState
                terminalController: root.terminalController
                helpHost: root.helpHost
                onConfigRequested: masterConfigDialog.open()
                onTerminalActivated: {
                    if (routerPanel && routerPanel.releaseTerminalFocus) {
                        routerPanel.releaseTerminalFocus()
                    }
                }
            }

            RouterPanel {
                id: routerPanel
                Layout.preferredWidth: root.shellState.masterRouterCollapsed
                                       ? routerPanel.collapsedRailWidth
                                       : Math.max(420, Math.round(root.width * 0.44))
                Layout.minimumWidth: root.shellState.masterRouterCollapsed
                                     ? routerPanel.collapsedRailWidth
                                     : 360
                Layout.maximumWidth: root.shellState.masterRouterCollapsed
                                     ? routerPanel.collapsedRailWidth
                                     : Math.max(520, Math.round(root.width * 0.52))
                Layout.fillHeight: true
                appState: root.appState
                routerState: root.routerState
                routerTerminalController: root.routerTerminalController
                collapsed: root.shellState.masterRouterCollapsed
                onCollapseToggled: root.shellState.masterRouterCollapsed = !root.shellState.masterRouterCollapsed
                onTerminalActivated: {
                    if (masterSurface && masterSurface.releaseTerminalFocus) {
                        masterSurface.releaseTerminalFocus()
                    }
                }
            }
        }
    }
}
