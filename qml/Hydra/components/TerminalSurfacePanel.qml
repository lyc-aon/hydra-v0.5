pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import Hydra.Backend 1.0
import "../styles"

FocusScope {
    id: root

    required property AppState appState
    required property TerminalSurfaceController terminalController
    property bool showLaunchSafetyChip: true
    property var helpHost: null
    property bool sessionStartEnabled: false
    readonly property bool terminalBindingReady: Boolean(root.appState && root.terminalController)
    readonly property bool compactMode: width < 600
    readonly property bool denseMode: width < 480
    readonly property bool focusWithin: root.activeFocus
    readonly property string selectedSessionLabel: root.appState.selectedSessionName.length > 0
                                                   ? root.appState.selectedSessionName
                                                   : "No session selected"

    activeFocusOnTab: true
    Accessible.role: Accessible.Grouping
    Accessible.name: "Terminal surface " + root.selectedSessionLabel

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

    function syncSelectedSession() {
        if (!root.terminalBindingReady) {
            return
        }
        root.terminalController.bindSession(root.appState.selectedSessionId,
                                            root.appState.selectedSessionName,
                                            root.appState.selectedSessionProviderKey,
                                            root.appState.selectedSessionTmuxSessionName,
                                            root.appState.selectedSessionPaneId,
                                            root.appState.selectedSessionWorkingDirectory)
    }

    property string _lastFocusedSessionId: ""

    Component.onCompleted: {
        Qt.callLater(root.syncSelectedSession)
        _lastFocusedSessionId = root.appState ? root.appState.selectedSessionId : ""
    }

    Connections {
        target: root.appState
        enabled: root.terminalBindingReady

        function onSelectedSessionChanged() {
            root.syncSelectedSession()
            const currentId = root.appState.selectedSessionId
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
        spacing: HydraTheme.space8

        RowLayout {
            Layout.fillWidth: true
            Layout.minimumHeight: 34
            spacing: HydraTheme.space12

            Item {
                Layout.fillWidth: true
                implicitHeight: terminalTitle.implicitHeight

                Text {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.topMargin: 1
                    text: "TERMINAL"
                    color: HydraTheme.withAlpha(HydraTheme.accentPhosphor, 0.1)
                    font.family: HydraTheme.displayFamily
                    font.pixelSize: 24
                    font.bold: true
                    font.letterSpacing: 1.8
                    scale: 1.01
                }

                Text {
                    id: terminalTitle
                    anchors.left: parent.left
                    anchors.top: parent.top
                    text: "TERMINAL"
                    color: HydraTheme.textOnDark
                    font.family: HydraTheme.displayFamily
                    font.pixelSize: 24
                    font.bold: true
                    font.letterSpacing: 1.8
                }
            }

            InfoDotButton {
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: Math.round((terminalTitle.implicitHeight - 18) / 2)
                topicId: "terminal"
                briefText: "Embedded terminal surface for the selected live session."
                accessibleLabel: "Explain terminal surface"
                hoverHost: root.helpHost
                onHelpRequested: (topicId, source) => root.requestHelp(topicId, source)
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.minimumHeight: 22
            spacing: root.denseMode ? HydraTheme.space4 : HydraTheme.space6

            Text {
                Layout.fillWidth: true
                text: root.selectedSessionLabel + " // native terminal // F4 focus"
                color: HydraTheme.accentSteelBright
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                elide: Text.ElideRight
            }

            StatusChip {
                visible: root.showLaunchSafetyChip && root.appState.selectedSessionAvailable
                accessibleLabel: "Selected session posture " + root.appState.selectedSessionLaunchSafetyLabel
                toneColor: root.appState.selectedSessionLaunchSafetyTone === "danger"
                           ? HydraTheme.danger
                           : HydraTheme.accentReady
                textColor: toneColor
                fillOpacity: root.appState.selectedSessionLaunchSafetyTone === "danger" ? 0.12 : 0.08
                borderOpacity: root.appState.selectedSessionLaunchSafetyTone === "danger" ? 0.38 : 0.24
                minWidth: root.denseMode ? 110 : 124
                text: "[" + root.appState.selectedSessionLaunchSafetyLabel.toUpperCase() + "]"
            }

            StatusChip {
                toneColor: root.appState.selectedSessionAvailable
                           ? HydraTheme.accentReady
                           : HydraTheme.accentSteelBright
                textColor: root.appState.selectedSessionAvailable
                           ? HydraTheme.accentReady
                           : HydraTheme.textOnDarkMuted
                fillOpacity: 0.1
                borderOpacity: 0.32
                minWidth: root.denseMode ? 82 : 92
                text: root.terminalController.active
                      ? (root.appState.selectedSessionAvailable ? "[LIVE]" : "[OFFLINE]")
                      : "[NO TARGET]"
            }

            ActionChip {
                enabled: root.terminalController.active
                text: "[ATTACH]"
                toneColor: HydraTheme.warning
                accessibleLabel: "Open external tmux attach"
                onClicked: root.terminalController.openExternalAttach()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: root.denseMode ? HydraTheme.radius8 : HydraTheme.radius10
            color: HydraTheme.boardPanel
            border.width: 1
            border.color: HydraTheme.withAlpha(HydraTheme.borderDark, 0.82)
            clip: true

            FrameCorners {
                anchors.fill: parent
                lineColor: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.2)
            }

            Loader {
                id: terminalViewportLoader
                anchors.fill: parent
                anchors.margins: HydraTheme.space10
                active: root.sessionStartEnabled && root.terminalController.active
                visible: active

                sourceComponent: NativeTerminalViewport {
                    accessibleName: "Embedded terminal viewport"
                    controller: root.terminalController
                    activeSessionNames: root.appState.liveTmuxSessionNames
                    denseMode: root.denseMode
                    sessionStartEnabled: root.sessionStartEnabled
                    tmuxSessionName: root.appState.selectedSessionTmuxSessionName
                    workingDirectory: root.appState.selectedSessionWorkingDirectory
                    onSessionStartRequested: root.focusTerminalPanel()
                }
            }

            Item {
                anchors.fill: parent
                visible: root.terminalController.active && !root.sessionStartEnabled

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
                        text: "> TERMINAL STANDBY"
                        color: HydraTheme.textOnDark
                        font.family: HydraTheme.displayFamily
                        font.pixelSize: root.denseMode ? 18 : 22
                        font.bold: true
                        font.letterSpacing: 1.0
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: Math.min(parent.width, 360)
                        text: "The session is selected but the embedded tmux attach stays unloaded until you click here or press F4."
                        color: HydraTheme.textOnDarkMuted
                        font.family: HydraTheme.bodyFamily
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            // Empty state — logo + text, hidden when terminal active
            Column {
                id: emptyState
                anchors.centerIn: parent
                spacing: HydraTheme.space20
                visible: !root.terminalController.active

                // Logo with radial glow
                Item {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 280
                    height: 280

                    HydraLogo {
                        anchors.centerIn: parent
                        logoSize: 280
                        tintColor: HydraTheme.accentBronze
                        opacity: 0.35
                    }

                    Rectangle {
                        anchors.centerIn: parent
                        width: 400; height: 400; radius: 200
                        color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.12)
                        z: -1
                    }
                    Rectangle {
                        anchors.centerIn: parent
                        width: 340; height: 340; radius: 170
                        color: HydraTheme.withAlpha(HydraTheme.accentSignal, 0.08)
                        z: -1
                    }
                }

                // Text
                Column {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: HydraTheme.space6
                    width: Math.min(emptyState.parent.width - (HydraTheme.space32 * 2), 420)

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "> NO ACTIVE TERMINAL TARGET"
                        color: HydraTheme.textOnDark
                        font.family: HydraTheme.displayFamily
                        font.pixelSize: root.denseMode ? 20 : 26
                        font.bold: true
                        font.letterSpacing: 1.1
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Select a live session from the board to bind the embedded terminal surface."
                        color: HydraTheme.textOnDarkMuted
                        font.family: HydraTheme.monoFamily
                        font.pixelSize: 10
                        wrapMode: Text.WordWrap
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            visible: root.terminalController.errorMessage.length > 0
            implicitHeight: footerText.implicitHeight + HydraTheme.space12
            radius: HydraTheme.radius6
            color: HydraTheme.withAlpha(HydraTheme.warning, 0.08)
            border.width: 1
            border.color: HydraTheme.withAlpha(HydraTheme.warning, 0.22)

            Text {
                id: footerText
                anchors.fill: parent
                anchors.margins: HydraTheme.space6
                text: root.terminalController.errorMessage
                color: HydraTheme.textOnDark
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                wrapMode: Text.WordWrap
            }
        }
    }
}
