pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import "../styles"

Item {
    id: root

    required property QtObject appState
    property QtObject helpHost: null
    property bool denseMode: false
    property bool tightMode: false

    function requestHelp(topicId, sourceItem) {
        if (helpHost && helpHost.openQuickHelp) {
            helpHost.openQuickHelp(topicId, sourceItem)
        }
    }

    implicitHeight: denseMode ? 62 : 76

    InfoDotButton {
        anchors.right: parent.right
        anchors.top: parent.top
        topicId: "console-overview"
        briefText: "Hydra is currently a repo/worktree launcher and detached tmux session register."
        accessibleLabel: "Explain console overview"
        hoverHost: root.helpHost
        onHelpRequested: (topicId, source) => root.requestHelp(topicId, source)
    }

    Text {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 1
        text: "HYDRA"
        color: HydraTheme.withAlpha(HydraTheme.accentPhosphor, 0.1)
        font.family: HydraTheme.displayFamily
        font.pixelSize: root.denseMode ? 22 : 30
        font.bold: true
        font.letterSpacing: 2.4
        scale: 1.01
    }

    Text {
        anchors.left: parent.left
        anchors.top: parent.top
        text: "HYDRA"
        color: HydraTheme.textOnDark
        font.family: HydraTheme.displayFamily
        font.pixelSize: root.denseMode ? 22 : 30
        font.bold: true
        font.letterSpacing: 2.4
    }

    Text {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: root.denseMode ? 32 : 42
        text: "ORCHESTRATION CONSOLE // TMUX GRID"
        color: HydraTheme.accentSteelBright
        font.family: HydraTheme.monoFamily
        font.pixelSize: 9
        font.bold: true
        opacity: 0.8
    }

    Row {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: root.denseMode ? 46 : 58
        spacing: HydraTheme.space6

        StatusChip {
            width: root.tightMode ? 58 : 72
            height: 18
            hoverHost: root.helpHost
            toneColor: root.appState.tmuxAvailable ? HydraTheme.accentReady : HydraTheme.danger
            textColor: root.appState.tmuxAvailable ? HydraTheme.accentReady : HydraTheme.danger
            fillOpacity: 0.1
            borderOpacity: 0.35
            text: root.appState.tmuxAvailable ? "[MUX]" : "[NO MUX]"
            toolTipText: "MUX reports whether tmux is available for detached shell launch."
        }

        StatusChip {
            minWidth: 54
            hoverHost: root.helpHost
            toneColor: HydraTheme.accentSteelBright
            textColor: HydraTheme.accentSteelBright
            text: "R:" + root.appState.repoCount
            toolTipText: "R is the number of repositories currently registered in Hydra."
        }

        StatusChip {
            minWidth: 54
            hoverHost: root.helpHost
            toneColor: HydraTheme.accentBronze
            textColor: HydraTheme.accentBronze
            borderOpacity: 0.26
            text: "W:" + root.appState.worktreeCount
            toolTipText: "W is the number of linked worktrees currently loaded for the selected repository."
        }
    }
}
