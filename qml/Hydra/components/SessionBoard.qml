pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import QtQuick.Controls 6.5
import "../styles"

Item {
    id: root

    required property QtObject appState
    property real layoutWidth: width
    property QtObject helpHost: null
    readonly property real effectiveWidth: layoutWidth > 0 ? layoutWidth : width
    readonly property bool compactMode: effectiveWidth < 1060
    readonly property bool denseMode: effectiveWidth < 900
    readonly property bool narrowMode: effectiveWidth < 760
    readonly property int boardInset: root.denseMode ? HydraTheme.space10 : HydraTheme.space12

    function requestHelp(topicId, sourceItem) {
        if (helpHost && helpHost.openQuickHelp) {
            helpHost.openQuickHelp(topicId, sourceItem)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: root.denseMode ? HydraTheme.space8 : HydraTheme.space10

        RowLayout {
            Layout.fillWidth: true
            spacing: HydraTheme.space12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: HydraTheme.space2

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
                        font.pixelSize: root.narrowMode ? 20 : (root.compactMode ? 24 : 28)
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
                        font.pixelSize: root.narrowMode ? 20 : (root.compactMode ? 24 : 28)
                        font.bold: true
                        font.letterSpacing: 1.8
                    }
                }

                Text {
                    visible: !root.compactMode
                    text: "channel register // detached tmux control"
                    color: HydraTheme.accentSteelBright
                    font.family: HydraTheme.monoFamily
                    font.pixelSize: 10
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }

            InfoDotButton {
                topicId: "sessions"
                briefText: "Active detached tmux session register, refresh path, and explicit end control."
                accessibleLabel: "Explain sessions"
                hoverHost: root.helpHost
                onHelpRequested: (topicId, source) => root.requestHelp(topicId, source)
            }

            Rectangle {
                id: refreshButton

                property bool hovered: false

                radius: HydraTheme.radius6
                color: hovered
                       ? HydraTheme.withAlpha(HydraTheme.accentSteel, 0.12)
                       : HydraTheme.withAlpha(HydraTheme.accentSteel, 0.03)
                border.width: 1
                border.color: hovered ? HydraTheme.borderFocus : HydraTheme.borderDark
                implicitWidth: root.narrowMode ? 76 : 96
                implicitHeight: root.denseMode ? 30 : 34
                scale: refreshArea.pressed ? 0.97 : 1.0
                transformOrigin: Item.Center
                Accessible.role: Accessible.Button
                Accessible.name: "Refresh Hydra state"
                Accessible.onPressAction: root.appState.refresh()

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
                    id: refreshArea

                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onEntered: {
                        refreshButton.hovered = true
                        if (root.helpHost && root.helpHost.queueHoverHint) {
                            root.helpHost.queueHoverHint("Reload repository, worktree, SQLite, and tmux state.",
                                                         refreshButton)
                        }
                    }
                    onExited: {
                        refreshButton.hovered = false
                        if (root.helpHost && root.helpHost.clearHoverHint) {
                            root.helpHost.clearHoverHint(refreshButton)
                        }
                    }
                    onClicked: root.appState.refresh()
                }

                Text {
                    anchors.centerIn: parent
                    text: "[REFRESH]"
                    color: refreshButton.hovered ? HydraTheme.accentSteelBright : HydraTheme.textOnDark
                    font.family: HydraTheme.monoFamily
                    font.pixelSize: root.denseMode ? 9 : 10
                    font.bold: true
                    font.letterSpacing: 0.8
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: root.compactMode ? 2 : 3
            columnSpacing: HydraTheme.space8
            rowSpacing: HydraTheme.space8

            MetricTile {
                Layout.fillWidth: true
                Layout.columnSpan: root.compactMode ? 2 : 1
                compactMode: root.denseMode
                label: "TARGET"
                value: root.appState.selectedWorktreeBranch.length > 0
                       ? root.appState.selectedWorktreeBranch
                       : root.appState.selectedRepoName
                baseFontFamily: HydraTheme.displayFamily
            }

            MetricTile {
                Layout.fillWidth: true
                compactMode: root.denseMode
                label: "ACTIVE"
                value: root.appState.sessionCount
                monospaceValue: true
            }

            MetricTile {
                Layout.fillWidth: true
                compactMode: root.denseMode
                label: "MUX"
                value: root.appState.tmuxAvailable ? "READY" : "MISSING"
                valueColor: root.appState.tmuxAvailable ? HydraTheme.accentReady : HydraTheme.danger
                valueMonospace: true
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
            Layout.fillWidth: true
            Layout.fillHeight: true
            appState: root.appState
            hoverHost: root.helpHost
            compactMode: root.compactMode
            denseMode: root.denseMode
            narrowMode: root.narrowMode
            boardInset: root.boardInset
        }
    }
}
