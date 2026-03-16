pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import Hydra.Backend 1.0
import "../styles"

FocusScope {
    id: root

    required property AppState appState
    required property RouterState routerState
    required property TerminalSurfaceController routerTerminalController
    required property bool collapsed
    property bool sessionStartEnabled: false
    signal collapseToggled()
    signal terminalActivated()

    readonly property bool routerLive: root.routerState.sessionAvailable
    readonly property string stateTone: root.routerState.sessionStateTone
    readonly property string activityLabel: root.routerState.sessionActivityLabel
    readonly property bool approvalPending: root.routerState.sessionApprovalPending
    readonly property color stateColor: HydraTheme.sessionStateColor(root.stateTone)
    readonly property string heartbeatMode: root.stateTone === "error"
                                            ? "error"
                                            : (root.approvalPending
                                               || root.activityLabel === "ACTIVE"
                                               || root.activityLabel === "INPUT")
                                              ? "active"
                                              : "idle"
    readonly property string statusText: {
        if (!root.routerLive) return "OFFLINE"
        if (root.approvalPending) return "PENDING"
        if (root.activityLabel === "ACTIVE") return "ACTIVE"
        if (root.activityLabel === "INPUT") return "INPUT"
        if (root.stateTone === "error") return "ERROR"
        return "READY"
    }
    readonly property int collapsedRailWidth: 74
    readonly property int expandedRailWidth: 520
    readonly property bool focusWithin: root.activeFocus
    readonly property bool terminalBindingReady: Boolean(root.routerState && root.routerTerminalController)

    implicitWidth: root.collapsed ? root.collapsedRailWidth : root.expandedRailWidth
    activeFocusOnTab: true

    Accessible.role: Accessible.Grouping
    Accessible.name: "Router rail"

    RouterConfigDialog {
        id: routerConfigDialog
        appState: root.appState
        routerState: root.routerState
    }

    Connections {
        target: root.routerState
        enabled: root.terminalBindingReady

        function onStateChanged() {
            root.syncRouterTerminal()
        }
    }

    function syncRouterTerminal() {
        if (!root.terminalBindingReady) {
            return
        }
        root.routerTerminalController.bindSession(
            root.routerState.sessionId,
            root.routerState.sessionName,
            root.routerState.sessionProviderKey,
            root.routerState.sessionTmuxSessionName,
            root.routerState.sessionPaneId,
            root.routerState.sessionWorkingDirectory)
    }

    function focusPrimaryControl() {
        if (root.collapsed) {
            root.collapseToggled()
            Qt.callLater(root.focusPrimaryControl)
            return
        }
        if (root.routerLive) {
            root.sessionStartEnabled = true
            Qt.callLater(function() {
                if (routerTerminalLoader.item && routerTerminalLoader.item.focusTerminal) {
                    routerTerminalLoader.item.focusTerminal()
                }
            })
            return
        }
        configChip.forceActiveFocus()
    }

    function releaseTerminalFocus() {
        if (routerTerminalLoader.item && routerTerminalLoader.item.releaseTerminalFocus) {
            routerTerminalLoader.item.releaseTerminalFocus()
        }
    }

    Component.onCompleted: Qt.callLater(root.syncRouterTerminal)

    Rectangle {
        anchors.fill: parent
        radius: HydraTheme.radius10
        gradient: Gradient {
            GradientStop { position: 0.0; color: HydraTheme.withAlpha(HydraTheme.boardPanel, 0.98) }
            GradientStop { position: 1.0; color: HydraTheme.withAlpha(HydraTheme.chromeGlass, 0.92) }
        }
        border.width: 1
        border.color: HydraTheme.withAlpha(root.collapsed ? HydraTheme.accentSteelBright : root.stateColor,
                                           root.collapsed ? 0.24 : 0.3)
        clip: true

        FrameCorners {
            anchors.fill: parent
            lineColor: HydraTheme.withAlpha(root.collapsed ? HydraTheme.accentSteelBright : root.stateColor,
                                            0.16)
        }

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 2
            color: HydraTheme.withAlpha(root.collapsed ? HydraTheme.accentSteelBright : root.stateColor, 0.74)
        }

        Item {
            anchors.fill: parent
            visible: root.collapsed

            Column {
                anchors.fill: parent
                anchors.margins: HydraTheme.space8
                spacing: HydraTheme.space10

                Rectangle {
                    width: 28
                    height: 28
                    radius: 14
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: HydraTheme.withAlpha(root.stateColor, 0.16)
                    border.width: 1
                    border.color: HydraTheme.withAlpha(root.stateColor, 0.38)

                    Rectangle {
                        anchors.centerIn: parent
                        width: 10
                        height: 10
                        radius: 5
                        color: root.routerLive ? root.stateColor : HydraTheme.accentSteelBright
                        opacity: root.routerLive ? 1.0 : 0.42
                    }
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "R\nO\nU\nT\nE\nR"
                    color: HydraTheme.textOnDarkMuted
                    font.family: HydraTheme.displayFamily
                    font.pixelSize: 9
                    font.bold: true
                    font.letterSpacing: 2.2
                    horizontalAlignment: Text.AlignHCenter
                }

                Rectangle {
                    width: 22
                    height: 76
                    radius: 10
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: HydraTheme.withAlpha(HydraTheme.chromeGlass, 0.56)
                    border.width: 1
                    border.color: HydraTheme.withAlpha(root.stateColor, 0.24)

                    Column {
                        anchors.centerIn: parent
                        spacing: 4

                        Repeater {
                            model: 4

                            Rectangle {
                                required property int index
                                width: 6
                                height: index === 1 || index === 2 ? 18 : 8
                                radius: 2
                                color: HydraTheme.withAlpha(root.stateColor,
                                                            index === 1 || index === 2 ? 0.88 : 0.3)
                            }
                        }
                    }
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: root.routerLive
                          ? (root.routerState.sessionProviderKey.length > 0
                                ? root.routerState.sessionProviderKey.toUpperCase()
                                : root.statusText)
                          : "AI"
                    color: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.82)
                    font.family: HydraTheme.monoFamily
                    font.pixelSize: 8
                    font.bold: true
                    rotation: 90
                    transformOrigin: Item.Center
                }
            }

            TapHandler {
                onTapped: root.collapseToggled()
            }
        }

        Item {
            anchors.fill: parent
            visible: !root.collapsed

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: HydraTheme.space10
                spacing: HydraTheme.space8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: HydraTheme.space8

                    Item {
                        Layout.fillWidth: true
                        implicitHeight: routerTitle.implicitHeight

                        Text {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.topMargin: 1
                            text: "ROUTER TERMINAL"
                            color: HydraTheme.withAlpha(root.stateColor, 0.12)
                            font.family: HydraTheme.displayFamily
                            font.pixelSize: 22
                            font.bold: true
                            font.letterSpacing: 1.8
                            scale: 1.01
                        }

                        Text {
                            id: routerTitle
                            anchors.left: parent.left
                            anchors.top: parent.top
                            text: "ROUTER TERMINAL"
                            color: HydraTheme.textOnDark
                            font.family: HydraTheme.displayFamily
                            font.pixelSize: 22
                            font.bold: true
                            font.letterSpacing: 1.8
                        }
                    }

                    StatusChip {
                        text: "[" + root.statusText + "]"
                        toneColor: root.routerLive ? root.stateColor : HydraTheme.accentSteelBright
                        textColor: root.routerLive ? root.stateColor : HydraTheme.textOnDarkMuted
                        fillOpacity: 0.08
                        borderOpacity: 0.24
                        minWidth: 74
                    }

                    StatusChip {
                        visible: root.routerLive
                        text: "[" + root.routerState.sessionProviderKey + "]"
                        toneColor: HydraTheme.accentSteelBright
                        textColor: HydraTheme.textOnDarkMuted
                        fillOpacity: 0.05
                        borderOpacity: 0.16
                        minWidth: 76
                    }

                    ActionChip {
                        visible: !root.routerLive && root.routerState.providerKey.length > 0
                        text: "[LAUNCH]"
                        toneColor: HydraTheme.accentReady
                        accessibleLabel: "Launch router session"
                        minWidth: 58
                        onClicked: {
                            root.sessionStartEnabled = true
                            Qt.callLater(function() {
                                root.routerState.ensureSession()
                            })
                        }
                    }

                    ActionChip {
                        id: configChip
                        text: "[CONFIG]"
                        toneColor: HydraTheme.accentSteelBright
                        accessibleLabel: "Configure router session"
                        minWidth: 58
                        onClicked: routerConfigDialog.open()
                    }

                    ActionChip {
                        text: "[>>]"
                        toneColor: HydraTheme.accentSteelBright
                        accessibleLabel: "Collapse router rail"
                        minWidth: 44
                        onClicked: root.collapseToggled()
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 40
                    radius: HydraTheme.radius8
                    color: HydraTheme.withAlpha(HydraTheme.chromeGlass, 0.52)
                    border.width: 1
                    border.color: HydraTheme.withAlpha(root.stateColor, 0.26)

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: HydraTheme.space8
                        spacing: HydraTheme.space8

                        HeartbeatTrace {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            mode: root.heartbeatMode
                            traceColor: root.routerLive
                                        ? (root.stateTone === "error" ? HydraTheme.danger : root.stateColor)
                                        : HydraTheme.accentSteelBright
                        }

                        Text {
                            Layout.preferredWidth: 156
                            text: root.routerLive
                                  ? ("smart router // " + root.statusText.toLowerCase())
                                  : (root.routerState.providerKey.length > 0
                                        ? "smart router // launch when needed"
                                        : "smart router // configure a provider")
                            color: HydraTheme.textOnDarkMuted
                            font.family: HydraTheme.monoFamily
                            font.pixelSize: 9
                            font.letterSpacing: 0.6
                            wrapMode: Text.WordWrap
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: HydraTheme.radius8
                    color: HydraTheme.withAlpha(HydraTheme.boardPanelMuted, 0.88)
                    border.width: 1
                    border.color: HydraTheme.withAlpha(root.stateColor, 0.2)
                    clip: true

                    Loader {
                        id: routerTerminalLoader
                        anchors.fill: parent
                        anchors.margins: HydraTheme.space8
                        active: root.routerLive && root.sessionStartEnabled
                        visible: active

                        sourceComponent: NativeTerminalViewport {
                            accessibleName: "Router terminal viewport"
                            controller: root.routerTerminalController
                            activeSessionNames: root.appState.liveTmuxSessionNames
                            denseMode: true
                            sessionStartEnabled: root.sessionStartEnabled
                            tmuxSessionName: root.routerState.sessionTmuxSessionName
                            workingDirectory: root.routerState.sessionWorkingDirectory
                            onSessionStartRequested: root.focusPrimaryControl()
                            onTerminalActivated: root.terminalActivated()
                        }
                    }

                    Item {
                        anchors.fill: parent
                        visible: root.routerLive && !root.sessionStartEnabled

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.focusPrimaryControl()
                        }

                        Column {
                            anchors.centerIn: parent
                            spacing: HydraTheme.space6

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: "ROUTER LINK STANDBY"
                                color: HydraTheme.textOnDark
                                font.family: HydraTheme.displayFamily
                                font.pixelSize: 16
                                font.bold: true
                                font.letterSpacing: 1.0
                            }

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: Math.min(parent.width, 280)
                                text: "The router stays unloaded until you click here or focus the pane."
                                color: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.84)
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
                        visible: !root.routerLive

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "SMART ROUTER OFFLINE"
                            color: HydraTheme.textOnDarkMuted
                            font.family: HydraTheme.displayFamily
                            font.pixelSize: 14
                            font.bold: true
                            font.letterSpacing: 1.2
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: root.routerState.providerKey.length > 0
                                  ? "The router is configured but offline. Launch it when you want the smart router active."
                                  : "Configure an AI provider for the router, then launch it when you need it."
                            color: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.8)
                            font.family: HydraTheme.bodyFamily
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            width: Math.min(260, parent.width - HydraTheme.space16)
                            horizontalAlignment: Text.AlignHCenter
                        }

                        ActionChip {
                            anchors.horizontalCenter: parent.horizontalCenter
                            visible: root.routerState.providerKey.length > 0
                            text: "[LAUNCH ROUTER]"
                            toneColor: HydraTheme.accentReady
                            minWidth: 126
                            accessibleLabel: "Launch router terminal"
                            onClicked: {
                                root.sessionStartEnabled = true
                                Qt.callLater(function() {
                                    root.routerState.ensureSession()
                                })
                            }
                        }
                    }
                }
            }
        }
    }
}
