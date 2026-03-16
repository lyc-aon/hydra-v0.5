pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import QtQuick.Controls 6.5
import Hydra.Backend 1.0
import "../styles"

FocusScope {
    id: root

    required property AppState appState
    property bool showLaunchSafetyChips: true
    property real layoutWidth: width
    property var helpHost: null
    property string startupExpandedSessionName: ""
    readonly property real effectiveWidth: layoutWidth > 0 ? layoutWidth : width
    readonly property bool compactMode: effectiveWidth < 1060
    readonly property bool denseMode: effectiveWidth < 900
    readonly property bool narrowMode: effectiveWidth < 760
    readonly property bool focusWithin: root.activeFocus
    readonly property int boardInset: root.denseMode ? HydraTheme.space10 : HydraTheme.space12
    readonly property string selectedTargetLabel: root.appState.selectedWorktreeBranch.length > 0
                                                  ? root.appState.selectedWorktreeBranch
                                                  : root.appState.selectedRepoName
    signal sessionActivated(string sessionId)

    activeFocusOnTab: true

    function requestHelp(topicId, sourceItem) {
        if (helpHost && helpHost.openQuickHelp) {
            helpHost.openQuickHelp(topicId, sourceItem)
        }
    }

    function focusBoard() {
        sessionsSurface.focusBoard()
    }

    function selectNextSession() {
        sessionsSurface.selectNextSession()
    }

    function selectPreviousSession() {
        sessionsSurface.selectPreviousSession()
    }

    function toggleSelectedTrace() {
        sessionsSurface.toggleSelectedTrace()
    }

    function terminateSelectedSession() {
        sessionsSurface.terminateSelectedSession()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: HydraTheme.space8

        RowLayout {
            Layout.fillWidth: true
            Layout.minimumHeight: 34
            spacing: HydraTheme.space12

            Item {
                Layout.fillWidth: true
                implicitHeight: headerText.implicitHeight

                Text {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.topMargin: 1
                    text: "SESSIONS"
                    color: HydraTheme.withAlpha(HydraTheme.accentPhosphor, 0.1)
                    font.family: HydraTheme.displayFamily
                    font.pixelSize: 24
                    font.bold: true
                    font.letterSpacing: 1.8
                    scale: 1.01
                }

                Text {
                    id: headerText
                    anchors.left: parent.left
                    anchors.top: parent.top
                    text: "SESSIONS"
                    color: HydraTheme.textOnDark
                    font.family: HydraTheme.displayFamily
                    font.pixelSize: 24
                    font.bold: true
                    font.letterSpacing: 1.8
                }
            }

            InfoDotButton {
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: Math.round((headerText.implicitHeight - 18) / 2)
                topicId: "sessions"
                briefText: "Active detached tmux session register, refresh path, and explicit end control."
                accessibleLabel: "Explain sessions"
                hoverHost: root.helpHost
                onHelpRequested: (topicId, source) => root.requestHelp(topicId, source)
            }

            RowLayout {
                Layout.alignment: Qt.AlignTop
                spacing: HydraTheme.space8

                HydraButton {
                    implicitWidth: root.narrowMode ? 92 : 104
                    implicitHeight: root.denseMode ? 30 : 34
                    text: root.appState.sessionAutosortEnabled ? "[AUTO]" : "[MANUAL]"
                    accessibleLabel: root.appState.sessionAutosortEnabled
                                     ? "Disable session autosort"
                                     : "Enable session autosort"
                    toolTipText: root.appState.sessionAutosortEnabled
                                 ? "Hydra is grouping sessions automatically. Click to return to persistent manual order."
                                 : "Hydra is preserving your session order. Drag rows to reorder or click to enable autosort."
                    toneColor: root.appState.sessionAutosortEnabled
                               ? HydraTheme.accentSignal
                               : HydraTheme.accentBronze
                    active: root.appState.sessionAutosortEnabled
                    hoverHost: root.helpHost
                    onTriggered: root.appState.sessionAutosortEnabled = !root.appState.sessionAutosortEnabled
                }

                HydraButton {
                    implicitWidth: root.narrowMode ? 82 : 96
                    implicitHeight: root.denseMode ? 30 : 34
                    text: "[ALIAS]"
                    accessibleLabel: "Rename selected session alias"
                    toolTipText: root.appState.selectedSessionAvailable
                                 ? "Set or change the alias for the selected session. Aliases are used by master terminal routing (@alias)."
                                 : "Select a live session before setting its alias."
                    toneColor: HydraTheme.accentBronze
                    enabledState: root.appState.selectedSessionAvailable
                    hoverHost: root.helpHost
                    onTriggered: {
                        const alias = root.appState.selectedSessionAlias
                        root.openAliasEditor(root.appState.selectedSessionId, alias)
                    }
                }

                HydraButton {
                    implicitWidth: root.narrowMode ? 92 : 112
                    implicitHeight: root.denseMode ? 30 : 34
                    text: "[EXPORT]"
                    shortcutHint: "F10"
                    accessibleLabel: "Export selected session audit"
                    toolTipText: root.appState.selectedSessionAvailable
                                 ? "Write the selected session timeline and posture into a JSON audit."
                                 : "Select a live session before exporting its audit."
                    toneColor: HydraTheme.accentBronze
                    enabledState: root.appState.selectedSessionAvailable
                    hoverHost: root.helpHost
                    onTriggered: root.appState.exportSelectedSessionAudit()
                }

                HydraButton {
                    implicitWidth: root.narrowMode ? 96 : 116
                    implicitHeight: root.denseMode ? 30 : 34
                    text: root.appState.refreshing ? "[SYNCING]" : "[REFRESH]"
                    shortcutHint: "F5"
                    accessibleLabel: root.appState.refreshing ? "Refreshing Hydra state" : "Refresh Hydra state"
                    toolTipText: "Reload repository, worktree, SQLite, and tmux state."
                    toneColor: HydraTheme.accentSteelBright
                    active: root.appState.refreshing
                    enabledState: !root.appState.refreshing
                    hoverHost: root.helpHost
                    onTriggered: root.appState.refresh()
                }
            }
        }

        Text {
            visible: !root.compactMode
            text: root.appState.sessionAutosortEnabled
                  ? "active detached shells // autosort enabled // idle first, approvals second, active last // F3 focus // F8 trace // Shift+F8 end // F10 export"
                  : "active detached shells // manual order // drag rows to reorder // F3 focus // F8 trace // Shift+F8 end // F10 export"
            color: HydraTheme.accentSteelBright
            font.family: HydraTheme.monoFamily
            font.pixelSize: 10
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.minimumHeight: 22
            spacing: root.denseMode ? HydraTheme.space4 : HydraTheme.space6

            StatusChip {
                toneColor: HydraTheme.accentBronze
                textColor: root.appState.selectedRepoId.length > 0 ? HydraTheme.accentBronze : HydraTheme.textOnDarkMuted
                fillOpacity: 0.08
                borderOpacity: 0.24
                minWidth: root.denseMode ? 130 : 182
                text: root.appState.selectedRepoId.length > 0
                      ? "[TARGET " + root.selectedTargetLabel.toUpperCase() + "]"
                      : "[NO TARGET]"
            }

            StatusChip {
                toneColor: root.appState.sessionCount > 0 ? HydraTheme.accentReady : HydraTheme.accentSteel
                textColor: root.appState.sessionCount > 0 ? HydraTheme.accentReady : HydraTheme.textOnDarkMuted
                fillOpacity: 0.1
                borderOpacity: 0.3
                minWidth: root.denseMode ? 82 : 92
                text: root.appState.sessionCount > 0
                      ? "[LIVE " + root.appState.sessionCount + "]"
                      : "[STANDBY]"
            }

            StatusChip {
                visible: !root.appState.tmuxAvailable
                toneColor: HydraTheme.danger
                textColor: HydraTheme.danger
                fillOpacity: 0.1
                borderOpacity: 0.32
                minWidth: root.denseMode ? 86 : 108
                text: "[MUX BLOCKED]"
            }

            Item {
                Layout.fillWidth: true
            }
        }

        Rectangle {
            id: statusBanner

            readonly property bool active: root.appState.statusMessage.length > 0
            readonly property color toneColor: root.appState.statusIsWarning
                                              ? HydraTheme.warning
                                              : HydraTheme.accentSteelBright

            Layout.fillWidth: true
            visible: active || opacity > 0.0
            opacity: active ? 1.0 : 0.0
            implicitHeight: Math.round((statusText.implicitHeight
                                        + (root.denseMode ? HydraTheme.space16 : HydraTheme.space20))
                                       * opacity)
            radius: HydraTheme.radius6
            color: HydraTheme.withAlpha(toneColor, root.appState.statusIsWarning ? 0.12 : 0.08)
            border.width: 1
            border.color: HydraTheme.withAlpha(toneColor, root.appState.statusIsWarning ? 0.34 : 0.24)
            clip: true
            scale: active ? 1.0 : 0.985
            Accessible.role: Accessible.StaticText
            Accessible.name: root.appState.statusMessage

            Behavior on opacity {
                NumberAnimation { duration: HydraTheme.motionNormal }
            }

            Behavior on scale {
                NumberAnimation {
                    duration: HydraTheme.motionNormal
                    easing.type: Easing.OutCubic
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 2
                color: HydraTheme.withAlpha(statusBanner.toneColor, 0.62)
            }

            Text {
                id: statusText

                anchors.fill: parent
                anchors.margins: HydraTheme.space10
                text: root.appState.statusMessage
                color: HydraTheme.textOnDark
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
            }
        }

        SessionsSurface {
            id: sessionsSurface
            Layout.fillWidth: true
            Layout.fillHeight: true
            appState: root.appState
            showLaunchSafetyChips: root.showLaunchSafetyChips
            hoverHost: root.helpHost
            startupExpandedSessionName: root.startupExpandedSessionName
            compactMode: root.compactMode
            denseMode: root.denseMode
            narrowMode: root.narrowMode
            boardInset: root.boardInset
            onSessionActivated: sessionId => root.sessionActivated(sessionId)
            onAliasEditRequested: (sessionId, currentAlias) => root.openAliasEditor(sessionId, currentAlias)
        }
    }

    function openAliasEditor(sessionId, currentAlias) {
        aliasEditDialog.targetSessionId = sessionId
        aliasEditDialog.initialAlias = currentAlias
        aliasEditDialog.open()
    }

    Timer {
        id: aliasFieldFocusTimer
        interval: 80
        onTriggered: {
            aliasEditField.forceActiveFocus()
            aliasEditField.selectAll()
        }
    }

    Dialog {
        id: aliasEditDialog

        property string targetSessionId: ""
        property string initialAlias: ""

        parent: Overlay.overlay
        modal: true
        dim: true
        focus: true
        x: Math.round((root.width - width) * 0.5)
        y: Math.round((root.height - height) * 0.38)
        width: Math.min(360, root.width - (HydraTheme.space20 * 2))
        padding: 0
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        onOpened: {
            aliasEditField.text = aliasEditDialog.initialAlias
            aliasFieldFocusTimer.restart()
        }

        onClosed: aliasFieldFocusTimer.stop()

        function commitAlias() {
            const trimmed = aliasEditField.text.trim()
            if (trimmed.length > 0 && aliasEditDialog.targetSessionId.length > 0) {
                root.appState.setSessionAlias(aliasEditDialog.targetSessionId, trimmed)
            }
            aliasEditDialog.close()
        }

        background: Rectangle {
            color: HydraTheme.boardPanelMuted
            border.width: 1
            border.color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.5)
            radius: HydraTheme.radius10

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 2
                color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.6)
                radius: HydraTheme.radius10
            }
        }

        Overlay.modal: Rectangle {
            color: HydraTheme.withAlpha("#000000", 0.55)
        }

        contentItem: ColumnLayout {
            spacing: HydraTheme.space8

            Item { Layout.preferredHeight: HydraTheme.space12 }

            Text {
                text: "SESSION ALIAS"
                color: HydraTheme.accentBronze
                font.family: HydraTheme.monoFamily
                font.pixelSize: 11
                font.bold: true
                font.letterSpacing: 1.2
                Layout.leftMargin: HydraTheme.space16
                Layout.rightMargin: HydraTheme.space16
            }

            Text {
                text: "Set a short name for routing from the master terminal.\nMust be unique across live sessions."
                color: HydraTheme.textOnDarkMuted
                font.family: HydraTheme.bodyFamily
                font.pixelSize: 11
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                Layout.leftMargin: HydraTheme.space16
                Layout.rightMargin: HydraTheme.space16
            }

            Item { Layout.preferredHeight: HydraTheme.space4 }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: HydraTheme.space16
                Layout.rightMargin: HydraTheme.space16
                implicitHeight: 36
                color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.06)
                border.width: 1
                border.color: aliasEditField.activeFocus
                              ? HydraTheme.withAlpha(HydraTheme.accentBronze, 0.6)
                              : HydraTheme.withAlpha(HydraTheme.accentBronze, 0.24)
                radius: HydraTheme.radius6

                Behavior on border.color {
                    ColorAnimation { duration: HydraTheme.motionFast }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: HydraTheme.space10
                    anchors.rightMargin: HydraTheme.space10
                    spacing: 2

                    Text {
                        text: "@"
                        color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.5)
                        font.family: HydraTheme.monoFamily
                        font.pixelSize: 14
                        font.bold: true
                    }

                    TextInput {
                        id: aliasEditField

                        Layout.fillWidth: true
                        color: HydraTheme.accentBronze
                        selectionColor: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.3)
                        selectedTextColor: HydraTheme.textOnLight
                        font.family: HydraTheme.monoFamily
                        font.pixelSize: 14
                        font.bold: true
                        clip: true
                        verticalAlignment: TextInput.AlignVCenter

                        onAccepted: aliasEditDialog.commitAlias()
                        Keys.onEscapePressed: aliasEditDialog.close()
                    }
                }
            }

            Item { Layout.preferredHeight: HydraTheme.space4 }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: HydraTheme.space16
                Layout.rightMargin: HydraTheme.space16
                spacing: HydraTheme.space8

                Item { Layout.fillWidth: true }

                ActionChip {
                    implicitWidth: 72
                    implicitHeight: 26
                    text: "[CANCEL]"
                    toneColor: HydraTheme.accentSteelBright
                    textPixelSize: 10
                    accessibleLabel: "Cancel alias edit"
                    onClicked: aliasEditDialog.close()
                }

                ActionChip {
                    implicitWidth: 72
                    implicitHeight: 26
                    text: "[SET]"
                    toneColor: HydraTheme.accentBronze
                    textPixelSize: 10
                    accessibleLabel: "Confirm alias"
                    onClicked: aliasEditDialog.commitAlias()
                }
            }

            Item { Layout.preferredHeight: HydraTheme.space12 }
        }
    }
}
