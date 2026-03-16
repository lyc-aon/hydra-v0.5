pragma ComponentBehavior: Bound
import QtQuick 6.5

Item {
    id: root

    required property var host

    visible: false

    Shortcut { sequence: "F2"; context: Qt.ApplicationShortcut; enabled: root.host.shellShortcutsEnabled; onActivated: root.host.focusRailPane() }
    Shortcut { sequence: "F3"; context: Qt.ApplicationShortcut; enabled: root.host.shellShortcutsEnabled; onActivated: root.host.focusSessionPane() }
    Shortcut { sequence: "F4"; context: Qt.ApplicationShortcut; enabled: root.host.shellShortcutsEnabled; onActivated: root.host.focusTerminalPane() }
    Shortcut { sequence: "Shift+F4"; context: Qt.ApplicationShortcut; enabled: root.host.shellShortcutsEnabled; onActivated: root.host.focusRouterPane() }
    Shortcut { sequence: "F5"; context: Qt.ApplicationShortcut; enabled: root.host.shellShortcutsEnabled; onActivated: root.host.appState.refresh() }
    Shortcut { sequence: "F6"; context: Qt.ApplicationShortcut; enabled: root.host.shellShortcutsEnabled; onActivated: root.host.cycleFocusZone(false) }
    Shortcut { sequence: "Shift+F6"; context: Qt.ApplicationShortcut; enabled: root.host.shellShortcutsEnabled; onActivated: root.host.cycleFocusZone(true) }
    Shortcut { sequence: "F7"; context: Qt.ApplicationShortcut; enabled: root.host.shellShortcutsEnabled; onActivated: root.host.appState.launchSelectedRepoSession() }
    Shortcut {
        sequence: "F8"
        context: Qt.ApplicationShortcut
        enabled: root.host.shellShortcutsEnabled
        onActivated: {
            if (root.host.activeViewMode === "workbench" && root.host.workbenchPane) {
                root.host.workbenchPane.toggleSelectedSessionTrace()
            }
        }
    }
    Shortcut { sequence: "F9"; context: Qt.ApplicationShortcut; enabled: root.host.shellShortcutsEnabled; onActivated: root.host.toggleViewMode() }
    Shortcut {
        sequence: "Shift+F8"
        context: Qt.ApplicationShortcut
        enabled: root.host.shellShortcutsEnabled
        onActivated: {
            if (root.host.workbenchPane) {
                root.host.workbenchPane.terminateSelectedSession()
            }
        }
    }
    Shortcut { sequence: "F10"; context: Qt.ApplicationShortcut; enabled: root.host.shellShortcutsEnabled; onActivated: root.host.appState.exportSelectedSessionAudit() }
    Shortcut { sequence: "F11"; context: Qt.ApplicationShortcut; enabled: root.host.shellShortcutsEnabled; onActivated: root.host.toggleBorderlessFullscreen() }
    Shortcut { sequence: "F12"; context: Qt.ApplicationShortcut; enabled: root.host.shellShortcutsEnabled; onActivated: root.host.toggleSidebar() }
    Shortcut { sequence: "Ctrl+Shift+Up"; enabled: root.host.shellShortcutsEnabled; onActivated: root.host.appState.selectAdjacentSession(-1) }
    Shortcut { sequence: "Ctrl+Shift+Down"; enabled: root.host.shellShortcutsEnabled; onActivated: root.host.appState.selectAdjacentSession(1) }
    Shortcut { sequence: "Ctrl+Shift+Backspace"; enabled: root.host.shellShortcutsEnabled; onActivated: root.host.terminalController.killInputLine() }
    Shortcut {
        sequence: "Escape"
        enabled: root.host.startupOverlayActive
                 || root.host.detailHelpTopicId.length > 0
                 || root.host.quickHelpTopicId.length > 0
        onActivated: {
            if (root.host.startupOverlayActive) {
                root.host.skipStartupSequence()
                return
            }
            if (root.host.detailHelpTopicId.length > 0) {
                root.host.closeDetailHelp()
                return
            }
            if (root.host.quickHelpTopicId.length > 0) {
                root.host.closeQuickHelp()
            }
        }
    }
}
