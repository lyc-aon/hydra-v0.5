pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import Hydra.Backend 1.0
import "../styles"

FocusScope {
    id: root

    signal configRequested()
    signal terminalActivated()

    required property AppState appState
    required property MasterState masterState
    required property TerminalSurfaceController terminalController
    property var helpHost: null
    property bool sessionStartEnabled: false
    readonly property bool terminalBindingReady: Boolean(root.masterState && root.terminalController)
    readonly property bool denseMode: width < 480
    readonly property bool focusWithin: root.activeFocus
    readonly property string selectedAlias: root.masterState.sessionAlias.length > 0
                                            ? ("@" + root.masterState.sessionAlias)
                                            : ""
    readonly property string stateTone: root.masterState.sessionStateTone
    readonly property string activityLabel: root.masterState.sessionActivityLabel
    readonly property bool approvalPending: root.masterState.sessionApprovalPending
    readonly property color stateColor: HydraTheme.sessionStateColor(root.stateTone)
    readonly property string heartbeatMode: root.stateTone === "error"
                                            ? "error"
                                            : (root.approvalPending
                                               || root.activityLabel === "ACTIVE"
                                               || root.activityLabel === "INPUT")
                                              ? "active"
                                              : "idle"
    readonly property string statusText: {
        if (!root.masterState.sessionAvailable) return "OFFLINE"
        if (root.approvalPending) return "PENDING"
        if (root.activityLabel === "ACTIVE") return "ACTIVE"
        if (root.activityLabel === "INPUT") return "INPUT"
        if (root.stateTone === "error") return "ERROR"
        return "READY"
    }

    activeFocusOnTab: true
    Accessible.role: Accessible.Grouping
    Accessible.name: "Master Terminal surface"

    function requestHelp(topicId, sourceItem) {
        if (helpHost && helpHost.openQuickHelp) {
            helpHost.openQuickHelp(topicId, sourceItem)
        }
    }

    function focusTerminalPanel() {
        root.sessionStartEnabled = true
        Qt.callLater(function() {
            if (terminalViewportLoader.item && terminalViewportLoader.item.focusTerminal) {
                terminalViewportLoader.item.focusTerminal()
            }
        })
    }

    function releaseTerminalFocus() {
        if (terminalViewportLoader.item && terminalViewportLoader.item.releaseTerminalFocus) {
            terminalViewportLoader.item.releaseTerminalFocus()
        }
    }

    function syncMasterSession() {
        if (!root.terminalBindingReady) {
            return
        }
        root.terminalController.bindSession(root.masterState.sessionId,
                                            root.masterState.sessionName,
                                            root.masterState.sessionProviderKey,
                                            root.masterState.sessionTmuxSessionName,
                                            root.masterState.sessionPaneId,
                                            root.masterState.sessionWorkingDirectory)
    }

    property string _lastFocusedSessionId: ""

    Component.onCompleted: {
        Qt.callLater(root.syncMasterSession)
        _lastFocusedSessionId = root.masterState ? root.masterState.sessionId : ""
    }

    Connections {
        target: root.masterState
        enabled: root.terminalBindingReady

        function onStateChanged() {
            root.syncMasterSession()
            const currentId = root.masterState.sessionId
            if (root.visible && root.sessionStartEnabled && currentId.length > 0
                && currentId !== root._lastFocusedSessionId)
            {
                root._lastFocusedSessionId = currentId
                Qt.callLater(root.focusTerminalPanel)
                return
            }
            root._lastFocusedSessionId = currentId
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: HydraTheme.space6

        RowLayout {
            Layout.fillWidth: true
            Layout.minimumHeight: 34
            spacing: HydraTheme.space8

            Item {
                Layout.fillWidth: true
                implicitHeight: masterTitle.implicitHeight

                Text {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.topMargin: 1
                    text: "MASTER TERMINAL"
                    color: HydraTheme.withAlpha(root.stateColor, 0.12)
                    font.family: HydraTheme.displayFamily
                    font.pixelSize: 22
                    font.bold: true
                    font.letterSpacing: 1.8
                    scale: 1.01
                }

                Text {
                    id: masterTitle
                    anchors.left: parent.left
                    anchors.top: parent.top
                    text: "MASTER TERMINAL"
                    color: HydraTheme.textOnDark
                    font.family: HydraTheme.displayFamily
                    font.pixelSize: 22
                    font.bold: true
                    font.letterSpacing: 1.8
                }
            }

            StatusChip {
                visible: root.selectedAlias.length > 0
                text: root.selectedAlias
                toneColor: HydraTheme.accentBronze
                textColor: HydraTheme.accentBronze
                fillOpacity: 0.12
                borderOpacity: 0.32
                minWidth: 56
            }

            StatusChip {
                visible: root.masterState.sessionAvailable
                text: "[" + root.statusText + "]"
                toneColor: root.stateColor
                textColor: root.stateColor
                fillOpacity: 0.08
                borderOpacity: 0.26
                minWidth: 72
            }

            StatusChip {
                visible: root.masterState.sessionAvailable
                text: "[" + root.masterState.sessionName + "]"
                toneColor: root.stateColor
                textColor: HydraTheme.textOnDarkMuted
                fillOpacity: 0.06
                borderOpacity: 0.18
                minWidth: 80
            }

            StatusChip {
                visible: root.masterState.sessionAvailable
                text: "[" + root.masterState.sessionProviderKey + "]"
                toneColor: HydraTheme.accentSteelBright
                textColor: HydraTheme.textOnDarkMuted
                fillOpacity: 0.05
                borderOpacity: 0.16
                minWidth: 76
            }

            ActionChip {
                visible: !root.masterState.sessionAvailable
                text: "[LAUNCH]"
                toneColor: HydraTheme.accentReady
                accessibleLabel: "Launch master session"
                minWidth: 62
                onClicked: {
                    root.sessionStartEnabled = true
                    Qt.callLater(function() {
                        root.masterState.ensureSession()
                    })
                }
            }

            ActionChip {
                text: "[CONFIG]"
                toneColor: HydraTheme.accentBronze
                accessibleLabel: "Configure master session"
                minWidth: 60
                onClicked: root.configRequested()
            }

            InfoDotButton {
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: Math.round((masterTitle.implicitHeight - 18) / 2)
                topicId: "master-terminal"
                briefText: "Full-width orchestration terminal for multi-session management."
                accessibleLabel: "Explain master terminal"
                hoverHost: root.helpHost
                onHelpRequested: (topicId, source) => root.requestHelp(topicId, source)
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 40
            radius: HydraTheme.radius8
            color: HydraTheme.withAlpha(HydraTheme.chromeGlass, 0.5)
            border.width: 1
            border.color: HydraTheme.withAlpha(root.stateColor, 0.28)

            RowLayout {
                anchors.fill: parent
                anchors.margins: HydraTheme.space8
                spacing: HydraTheme.space8

                HeartbeatTrace {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    mode: root.heartbeatMode
                    traceColor: root.stateTone === "error"
                                ? HydraTheme.danger
                                : root.stateColor
                }

                Text {
                    Layout.preferredWidth: root.denseMode ? 92 : 134
                    text: root.masterState.sessionAvailable
                          ? ("master channel // " + root.statusText.toLowerCase())
                          : "master channel // offline"
                    color: HydraTheme.textOnDarkMuted
                    font.family: HydraTheme.monoFamily
                    font.pixelSize: 9
                    font.letterSpacing: 0.6
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: root.denseMode ? HydraTheme.radius8 : HydraTheme.radius10
            gradient: Gradient {
                GradientStop { position: 0.0; color: HydraTheme.withAlpha(HydraTheme.boardPanel, 0.98) }
                GradientStop { position: 1.0; color: HydraTheme.withAlpha(HydraTheme.chromeGlass, 0.92) }
            }
            border.width: 1
            border.color: HydraTheme.withAlpha(root.stateColor, 0.26)
            clip: true

            FrameCorners {
                anchors.fill: parent
                lineColor: HydraTheme.withAlpha(root.stateColor, 0.18)
            }

            Rectangle {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 2
                color: HydraTheme.withAlpha(root.stateColor, 0.72)
            }

            Loader {
                id: terminalViewportLoader
                anchors.fill: parent
                anchors.margins: HydraTheme.space10
                active: root.sessionStartEnabled && root.masterState.sessionAvailable
                visible: active

                sourceComponent: NativeTerminalViewport {
                    accessibleName: "Master terminal viewport"
                    controller: root.terminalController
                    activeSessionNames: root.appState.liveTmuxSessionNames
                    denseMode: root.denseMode
                    sessionStartEnabled: root.sessionStartEnabled
                    tmuxSessionName: root.masterState.sessionTmuxSessionName
                    workingDirectory: root.masterState.sessionWorkingDirectory
                    onSessionStartRequested: root.focusTerminalPanel()
                    onTerminalActivated: root.terminalActivated()
                }
            }

            Item {
                anchors.fill: parent
                visible: root.masterState.sessionAvailable && !root.sessionStartEnabled

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.focusTerminalPanel()
                }

                Column {
                    anchors.centerIn: parent
                    spacing: HydraTheme.space8

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "MASTER LINK STANDBY"
                        color: HydraTheme.textOnDark
                        font.family: HydraTheme.displayFamily
                        font.pixelSize: root.denseMode ? 18 : 22
                        font.bold: true
                        font.letterSpacing: 1.0
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: Math.min(parent.width, 360)
                        text: "The master session is live, but the embedded attach stays unloaded until you click here or focus the terminal."
                        color: HydraTheme.textOnDarkMuted
                        font.family: HydraTheme.bodyFamily
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            Column {
                anchors.centerIn: parent
                spacing: HydraTheme.space6
                visible: !root.masterState.sessionAvailable

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "MASTER TERMINAL OFFLINE"
                    color: HydraTheme.textOnDarkMuted
                    font.family: HydraTheme.displayFamily
                    font.pixelSize: 14
                    font.bold: true
                    font.letterSpacing: 1.2
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "The control-plane master is now opt-in. Launch it only when you actually need orchestration."
                    color: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.8)
                    font.family: HydraTheme.bodyFamily
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                    width: Math.min(320, parent.width - HydraTheme.space16)
                    horizontalAlignment: Text.AlignHCenter
                }

                ActionChip {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "[LAUNCH MASTER]"
                    toneColor: HydraTheme.accentReady
                    minWidth: 132
                    accessibleLabel: "Launch master terminal"
                    onClicked: {
                        root.sessionStartEnabled = true
                        Qt.callLater(function() {
                            root.masterState.ensureSession()
                        })
                    }
                }
            }

        }
    }
}
