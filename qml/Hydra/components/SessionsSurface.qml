pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import QtQuick.Controls 6.5
import "../styles"

Rectangle {
    id: root

    required property QtObject appState
    property QtObject hoverHost: null
    property bool compactMode: false
    property bool denseMode: false
    property bool narrowMode: false
    property int boardInset: HydraTheme.space12

    color: HydraTheme.boardPanel
    border.width: 1
    border.color: HydraTheme.borderDark
    radius: root.denseMode ? HydraTheme.radius8 : HydraTheme.radius10
    clip: true

    Repeater {
        model: Math.ceil(parent.width / 144)

        Rectangle {
            required property int index

            x: index * 144
            y: 0
            width: 1
            height: parent.height
            color: HydraTheme.withAlpha(HydraTheme.gridLine, 0.24)
        }
    }

    Repeater {
        model: Math.ceil(parent.height / 72)

        Rectangle {
            required property int index

            x: 0
            y: index * 72
            width: parent.width
            height: 1
            color: HydraTheme.withAlpha(HydraTheme.gridLine, 0.24)
        }
    }

    Rectangle {
        id: scanline
        x: 0
        y: -1
        width: parent.width
        height: 1
        color: HydraTheme.withAlpha(HydraTheme.accentReady, 0.18)

        NumberAnimation on y {
            running: HydraTheme.ambientEnabled && root.appState.sessionCount > 0
            loops: Animation.Infinite
            from: -1
            to: root.height + 1
            duration: HydraTheme.motionDrift
        }
    }

    FrameCorners {
        anchors.fill: parent
        lineColor: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.22)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.boardInset
        spacing: root.denseMode ? HydraTheme.space8 : HydraTheme.space10

        RowLayout {
            Layout.fillWidth: true
            spacing: HydraTheme.space6

            StatusChip {
                toneColor: root.appState.tmuxAvailable ? HydraTheme.accentReady : HydraTheme.danger
                textColor: root.appState.tmuxAvailable ? HydraTheme.accentReady : HydraTheme.danger
                fillOpacity: 0.1
                borderOpacity: 0.3
                minWidth: root.denseMode ? 76 : 88
                text: root.appState.tmuxAvailable ? "[MUX READY]" : "[MUX BLOCKED]"
            }

            StatusChip {
                toneColor: HydraTheme.accentBronze
                textColor: root.appState.selectedWorktreeBranch.length > 0 ? HydraTheme.accentBronze : HydraTheme.textOnDarkMuted
                fillOpacity: 0.08
                borderOpacity: 0.24
                minWidth: root.denseMode ? 84 : 102
                text: root.appState.selectedWorktreeBranch.length > 0
                      ? "[TGT " + root.appState.selectedWorktreeBranch.toUpperCase() + "]"
                      : "[NO TARGET]"
            }

            Item {
                Layout.fillWidth: true
            }

            Text {
                text: root.appState.sessionCount > 0 ? "LIVE:" + root.appState.sessionCount : "IDLE"
                color: root.appState.sessionCount > 0 ? HydraTheme.accentReady : HydraTheme.textOnDarkMuted
                font.family: HydraTheme.monoFamily
                font.pixelSize: 9
                font.bold: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: HydraTheme.space8

            Text {
                text: root.appState.sessionCount > 0 ? "ACTIVE SESSIONS" : "SYSTEM STANDBY"
                color: HydraTheme.accentBronze
                font.family: HydraTheme.displayFamily
                font.pixelSize: root.denseMode ? 13 : 14
                font.bold: true
                font.letterSpacing: 1.4
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 1
                color: HydraTheme.withAlpha(HydraTheme.borderDark, 0.85)
            }

            Text {
                text: root.appState.sessionCount > 0 ? root.appState.sessionCount + " LIVE" : "IDLE"
                color: root.appState.sessionCount > 0 ? HydraTheme.accentReady : HydraTheme.textOnDarkMuted
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                font.bold: true

                SequentialAnimation on opacity {
                    running: HydraTheme.ambientEnabled && root.appState.sessionCount > 0
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.55; duration: 780 }
                    NumberAnimation { to: 1.0; duration: 780 }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Column {
                anchors.left: parent.left
                anchors.top: parent.top
                width: Math.min(parent.width, root.narrowMode ? 300 : 440)
                spacing: HydraTheme.space6
                visible: root.appState.sessionCount === 0

                Text {
                    id: emptyHeadline
                    text: root.narrowMode ? "> STANDBY" : "> COMMAND CHANNEL IDLE"
                    color: HydraTheme.textOnDark
                    font.family: HydraTheme.displayFamily
                    font.pixelSize: root.compactMode ? 24 : 30
                    font.bold: true
                    font.letterSpacing: 1.2
                }

                Row {
                    spacing: 0

                    Text {
                        text: "_"
                        color: HydraTheme.accentBronze
                        font.family: HydraTheme.monoFamily
                        font.pixelSize: emptyHeadline.font.pixelSize

                        SequentialAnimation on opacity {
                            running: HydraTheme.ambientEnabled && root.appState.sessionCount === 0
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.0; duration: 220 }
                            NumberAnimation { to: 1.0; duration: 220 }
                            PauseAnimation { duration: 640 }
                        }
                    }
                }

                Text {
                    width: parent.width
                    text: root.narrowMode
                          ? "open the rail, choose a target, then launch a shell"
                          : "choose a repository or worktree, then launch a detached shell to start the first live session"
                    wrapMode: Text.WordWrap
                    color: HydraTheme.textOnDarkMuted
                    font.family: HydraTheme.monoFamily
                    font.pixelSize: 10
                }

                Repeater {
                    model: root.narrowMode ? 2 : 3

                    StatusChip {
                        required property int index

                        width: parent.width
                        height: 18
                        toneColor: index === 2 ? HydraTheme.accentReady : HydraTheme.borderDark
                        textColor: index === 2 ? HydraTheme.accentReady : HydraTheme.textOnDarkMuted
                        fillOpacity: 0.48
                        borderOpacity: 0.6
                        text: index === 0
                              ? "[01] choose target"
                              : (index === 1 ? "[02] arm launch bus" : "[03] monitor live channel")
                    }
                }
            }

            ListView {
                id: sessionList

                anchors.fill: parent
                spacing: root.denseMode ? HydraTheme.space6 : HydraTheme.space8
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                interactive: contentHeight > height
                model: root.appState.sessionModel
                visible: root.appState.sessionCount > 0

                add: Transition {
                    NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: HydraTheme.motionNormal }
                    NumberAnimation { property: "y"; from: 8; to: 0; duration: HydraTheme.motionNormal }
                }

                remove: Transition {
                    NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: HydraTheme.motionNormal }
                    NumberAnimation { property: "y"; from: 0; to: -8; duration: HydraTheme.motionNormal }
                }

                displaced: Transition {
                    NumberAnimation { properties: "x,y"; duration: HydraTheme.motionNormal }
                }

                delegate: Item {
                    required property string sessionId
                    required property string name
                    required property string repoName
                    required property string detailText
                    required property string stateLabel
                    required property string stateColor
                    required property string updatedAtText
                    required property bool canTerminate

                    width: sessionList.width
                    height: sessionCard.implicitHeight
                    implicitHeight: sessionCard.implicitHeight

                    SessionCard {
                        id: sessionCard

                        anchors.left: parent.left
                        anchors.right: parent.right
                        sessionId: parent.sessionId
                        sessionName: parent.name
                        repoName: parent.repoName
                        detailText: parent.detailText
                        stateLabel: parent.stateLabel
                        stateColor: parent.stateColor
                        updatedAtText: parent.updatedAtText
                        canTerminate: parent.canTerminate
                        hoverHost: root.hoverHost
                        compactMode: root.compactMode
                        denseMode: root.denseMode
                        onTerminateRequested: targetSessionId => root.appState.terminateSession(targetSessionId)
                    }
                }
            }
        }
    }
}
