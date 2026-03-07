pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import "../styles"

SurfacePanel {
    id: root

    required property QtObject appState
    property QtObject helpHost: null
    property bool denseMode: false
    property bool tightMode: false
    contentMargin: denseMode ? HydraTheme.space8 : HydraTheme.space10
    contentSpacing: denseMode ? HydraTheme.space6 : HydraTheme.space8

    function requestHelp(topicId, sourceItem) {
        if (helpHost && helpHost.openQuickHelp) {
            helpHost.openQuickHelp(topicId, sourceItem)
        }
    }

    SectionHeader {
        Layout.fillWidth: true
        title: "LAUNCH BUS"

        InfoDotButton {
            topicId: "launch-bus"
            briefText: "Readiness for detached generic-shell launch through tmux."
            accessibleLabel: "Explain launch bus"
            hoverHost: root.helpHost
            onHelpRequested: (topicId, source) => root.requestHelp(topicId, source)
        }

        StatusChip {
            toneColor: root.appState.tmuxAvailable ? HydraTheme.accentReady : HydraTheme.danger
            textColor: root.appState.tmuxAvailable ? HydraTheme.accentReady : HydraTheme.danger
            fillOpacity: 0.1
            borderOpacity: 0.3
            minWidth: 82
            text: root.appState.tmuxAvailable ? "MUX READY" : "MUX BLOCKED"
        }
    }

    RowLayout {
        spacing: HydraTheme.space8

        StatusChip {
            toneColor: HydraTheme.accentBronze
            textColor: HydraTheme.accentBronze
            fillOpacity: 0.08
            borderOpacity: 0.28
            minWidth: root.tightMode ? 62 : (root.denseMode ? 88 : 102)
            verticalPadding: HydraTheme.space6
            textFamily: HydraTheme.displayFamily
            textPixelSize: 11
            textLetterSpacing: 0.8
            text: root.denseMode ? "[SHELL]" : "[GENERIC SHELL]"
        }

        StatusChip {
            toneColor: HydraTheme.accentSteel
            textColor: HydraTheme.accentSteelBright
            fillOpacity: 0.08
            borderOpacity: 0.22
            minWidth: root.tightMode ? 44 : (root.denseMode ? 52 : 68)
            verticalPadding: HydraTheme.space6
            textFamily: HydraTheme.displayFamily
            textPixelSize: 11
            text: "[TMUX]"
        }
    }

    Text {
        visible: !root.denseMode
        text: root.appState.tmuxAvailable
              ? "launch a detached shell for the current target"
              : "tmux is unavailable; launch remains blocked"
        color: root.appState.tmuxAvailable ? HydraTheme.textOnDarkMuted : HydraTheme.danger
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
    }

    Text {
        visible: root.appState.selectedRepoId.length > 0 && !root.appState.repoLocalStateReady
        text: "workspace setup is unavailable for the current target"
        color: HydraTheme.warning
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
    }
}
