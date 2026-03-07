pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts 6.5
import "../styles"

SurfacePanel {
    id: root

    required property QtObject appState
    property QtObject helpHost: null
    property bool denseMode: false
    property bool tightMode: false
    property bool canLaunch: false
    contentMargin: denseMode ? HydraTheme.space8 : HydraTheme.space10
    contentSpacing: denseMode ? HydraTheme.space6 : HydraTheme.space8
    panelColor: HydraTheme.railPanelStrong
    panelBorderColor: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.22)

    function requestHelp(topicId, sourceItem) {
        if (helpHost && helpHost.openQuickHelp) {
            helpHost.openQuickHelp(topicId, sourceItem)
        }
    }

    SectionHeader {
        Layout.fillWidth: true
        title: "EXECUTE"

        InfoDotButton {
            topicId: "execute"
            briefText: "Launch the current repository or linked worktree as a detached tmux shell."
            accessibleLabel: "Explain execute"
            hoverHost: root.helpHost
            onHelpRequested: (topicId, source) => root.requestHelp(topicId, source)
        }

        StatusChip {
            toneColor: root.canLaunch ? HydraTheme.accentReady : HydraTheme.accentMuted
            textColor: root.canLaunch ? HydraTheme.accentReady : HydraTheme.textOnDarkMuted
            fillOpacity: 0.1
            borderOpacity: 0.3
            minWidth: root.canLaunch ? 72 : 88
            text: root.canLaunch ? "ARMED" : "NOT READY"
        }
    }

    Text {
        text: root.appState.selectedWorktreeBranch.length > 0
              ? root.appState.selectedWorktreeBranch
              : root.appState.selectedRepoName
        color: HydraTheme.textOnDark
        font.family: HydraTheme.displayFamily
        font.pixelSize: root.denseMode ? 14 : 16
        font.bold: true
        elide: Text.ElideRight
        Layout.fillWidth: true
    }

    Text {
        visible: !root.tightMode
        text: root.appState.selectedWorktreePath.length > 0 ? root.appState.selectedWorktreePath : root.appState.repositoryRootPath
        color: HydraTheme.textOnLightSoft
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        elide: Text.ElideMiddle
        Layout.fillWidth: true
    }

    Rectangle {
        id: launchButton

        property bool hovered: false

        Layout.fillWidth: true
        implicitHeight: root.denseMode ? 40 : 44
        radius: HydraTheme.radius6
        color: root.canLaunch
               ? (hovered
                  ? HydraTheme.withAlpha(HydraTheme.accentOrangeStrong, 0.16)
                  : HydraTheme.withAlpha(HydraTheme.accentOrangeStrong, 0.08))
               : HydraTheme.withAlpha(HydraTheme.accentMuted, 0.42)
        border.width: 1
        border.color: root.canLaunch
                      ? HydraTheme.withAlpha(HydraTheme.accentOrangeStrong, hovered ? 0.68 : 0.42)
                      : HydraTheme.borderDark
        clip: true
        scale: launchArea.pressed && root.canLaunch ? 0.98 : 1.0
        transformOrigin: Item.Center
        Accessible.role: Accessible.Button
        Accessible.name: "Launch tmux shell"
        Accessible.onPressAction: {
            if (root.canLaunch) {
                root.appState.launchSelectedRepoSession()
            }
        }

        Behavior on color {
            ColorAnimation { duration: HydraTheme.motionFast }
        }

        Behavior on scale {
            NumberAnimation {
                duration: HydraTheme.motionFast
                easing.type: Easing.OutCubic
            }
        }

        MouseArea {
            id: launchArea

            anchors.fill: parent
            enabled: root.canLaunch
            hoverEnabled: true
            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
            onEntered: {
                launchButton.hovered = true
                if (root.helpHost && root.helpHost.queueHoverHint) {
                    root.helpHost.queueHoverHint(root.canLaunch
                                                 ? "Start a detached tmux shell in the current repository or worktree."
                                                 : "Select a target and ensure tmux is available before launching.",
                                                 launchButton)
                }
                if (root.canLaunch && HydraTheme.ambientEnabled) {
                    launchSweep.restartSweep()
                }
            }
            onExited: {
                launchButton.hovered = false
                if (root.helpHost && root.helpHost.clearHoverHint) {
                    root.helpHost.clearHoverHint(launchButton)
                }
            }
            onClicked: root.appState.launchSelectedRepoSession()
        }

        Rectangle {
            id: launchSweep
            x: -parent.width
            width: parent.width * 0.18
            height: parent.height
            color: HydraTheme.withAlpha(HydraTheme.accentCream, 0.08)
            rotation: 0
            visible: root.canLaunch

            function restartSweep() {
                if (!root.canLaunch) {
                    return
                }
                x = -launchButton.width
                launchSweepAnimation.restart()
            }

            NumberAnimation {
                id: launchSweepAnimation
                target: launchSweep
                property: "x"
                from: -launchButton.width
                to: launchButton.width * 1.2
                duration: HydraTheme.motionAmbient
            }
        }

        onVisibleChanged: {
            if (visible && root.canLaunch && HydraTheme.ambientEnabled) {
                launchSweep.restartSweep()
            }
        }

        FrameCorners {
            anchors.fill: parent
            lineColor: HydraTheme.withAlpha(HydraTheme.shellBg, 0.32)
            spanLength: 16
            spanHeight: 12
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: HydraTheme.space10
            spacing: HydraTheme.space10

            Rectangle {
                Layout.preferredWidth: 2
                Layout.fillHeight: true
                color: root.canLaunch ? HydraTheme.accentOrangeStrong : HydraTheme.borderDark
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                Text {
                    text: root.tightMode ? "LAUNCH" : (root.denseMode ? "LAUNCH SHELL" : "LAUNCH TMUX SHELL")
                    color: root.canLaunch ? HydraTheme.accentOrangeStrong : HydraTheme.textOnDarkMuted
                    font.family: HydraTheme.displayFamily
                    font.pixelSize: 11
                    font.bold: true
                    font.letterSpacing: 1.0
                }

                Text {
                    visible: !root.denseMode
                    text: root.appState.tmuxAvailable ? "detached tmux session" : "tmux unavailable"
                    color: root.canLaunch
                           ? HydraTheme.withAlpha(HydraTheme.textOnDark, 0.7)
                           : HydraTheme.textOnDarkMuted
                    font.family: HydraTheme.monoFamily
                    font.pixelSize: 9
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }
        }
    }
}
