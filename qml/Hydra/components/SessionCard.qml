import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts 6.5
import "../styles"

Rectangle {
    id: root

    property string sessionId: ""
    property string sessionName: ""
    property string repoName: ""
    property string detailText: ""
    property string stateLabel: ""
    property string stateColor: "#4d7599"
    property string updatedAtText: ""
    property bool canTerminate: false
    property QtObject hoverHost: null
    property bool compactMode: false
    property bool denseMode: false

    signal terminateRequested(string sessionId)

    readonly property bool pulsingState: root.canTerminate || stateLabel === "Starting"
                                         || stateLabel === "Thinking"
                                         || stateLabel === "Running Tool"
                                         || stateLabel === "Awaiting Approval"
                                         || stateLabel === "Waiting For Input"
    readonly property int contentInset: root.denseMode ? HydraTheme.space8 : HydraTheme.space10

    implicitHeight: contentLayout.implicitHeight + (root.contentInset * 2)
    radius: root.denseMode ? HydraTheme.radius4 : HydraTheme.radius6
    color: root.canTerminate ? HydraTheme.boardRowStrong : HydraTheme.boardRow
    border.width: 1
    border.color: HydraTheme.borderDark
    clip: true

    Rectangle {
        anchors.left: edgeBar.right
        anchors.top: parent.top
        anchors.right: parent.right
        height: 1
        color: HydraTheme.withAlpha(root.stateColor, 0.22)
    }

    Rectangle {
        id: edgeBar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 3
        color: root.stateColor

        SequentialAnimation on opacity {
            running: HydraTheme.ambientEnabled && root.pulsingState
            loops: Animation.Infinite
            NumberAnimation { to: 0.45; duration: 880 }
            NumberAnimation { to: 1.0; duration: 880 }
        }
    }

    GridLayout {
        id: contentLayout

        anchors.fill: parent
        anchors.leftMargin: root.contentInset
        anchors.rightMargin: root.contentInset
        anchors.topMargin: root.contentInset
        anchors.bottomMargin: root.contentInset
        columns: (root.compactMode || root.denseMode) ? 1 : 2
        columnSpacing: root.denseMode ? HydraTheme.space8 : HydraTheme.space12
        rowSpacing: root.denseMode ? HydraTheme.space4 : HydraTheme.space6

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: HydraTheme.space4

            RowLayout {
                Layout.fillWidth: true
                spacing: HydraTheme.space8

                Text {
                    text: root.sessionName
                    color: HydraTheme.textOnLight
                    font.family: HydraTheme.displayFamily
                    font.pixelSize: root.denseMode ? 12 : 14
                    font.bold: true
                    font.letterSpacing: 0.7
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }

            Text {
                visible: root.compactMode || root.denseMode
                text: root.updatedAtText
                color: HydraTheme.textOnLightSoft
                font.pixelSize: root.denseMode ? 8 : 9
                font.family: HydraTheme.monoFamily
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Text {
                text: root.repoName
                color: HydraTheme.accentBronze
                font.family: HydraTheme.monoFamily
                font.pixelSize: root.denseMode ? 8 : 9
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Text {
                text: root.detailText.replace("  |  ", " // ")
                color: HydraTheme.textOnDarkMuted
                font.pixelSize: root.denseMode ? 8 : 9
                font.family: HydraTheme.monoFamily
                wrapMode: Text.WordWrap
                maximumLineCount: root.denseMode ? 1 : (root.compactMode ? 2 : 1)
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        ColumnLayout {
            Layout.alignment: (root.compactMode || root.denseMode) ? Qt.AlignLeft : Qt.AlignTop | Qt.AlignRight
            Layout.fillWidth: root.compactMode || root.denseMode
            spacing: root.denseMode ? HydraTheme.space4 : HydraTheme.space6

            RowLayout {
                Layout.alignment: (root.compactMode || root.denseMode) ? Qt.AlignLeft : Qt.AlignRight
                Layout.fillWidth: root.compactMode || root.denseMode
                spacing: root.denseMode ? HydraTheme.space6 : HydraTheme.space8

                Rectangle {
                    radius: HydraTheme.radius4
                    color: HydraTheme.withAlpha(root.stateColor, 0.09)
                    border.width: 1
                    border.color: HydraTheme.withAlpha(root.stateColor, 0.3)
                    implicitWidth: stateRow.implicitWidth + (root.denseMode ? 10 : 14)
                    implicitHeight: root.denseMode ? 20 : 22

                    RowLayout {
                        id: stateRow
                        anchors.centerIn: parent
                        spacing: 6

                        Rectangle {
                            implicitWidth: 6
                            implicitHeight: 6
                            radius: 3
                            color: root.stateColor

                            SequentialAnimation on opacity {
                                running: HydraTheme.ambientEnabled && root.pulsingState
                                loops: Animation.Infinite
                                NumberAnimation { to: 0.35; duration: HydraTheme.motionSlow }
                                NumberAnimation { to: 1.0; duration: HydraTheme.motionSlow }
                            }
                        }

                        Text {
                            text: root.stateLabel.toUpperCase()
                            color: root.stateColor
                            font.family: HydraTheme.monoFamily
                            font.pixelSize: root.denseMode ? 8 : 9
                            font.bold: true
                        }
                    }
                }

                Rectangle {
                    id: endButton

                    property bool hovered: false

                    visible: root.canTerminate
                    radius: HydraTheme.radius4
                    color: hovered
                           ? HydraTheme.withAlpha(HydraTheme.danger, 0.12)
                           : HydraTheme.withAlpha(HydraTheme.danger, 0.06)
                    border.width: 1
                    border.color: HydraTheme.withAlpha(HydraTheme.danger, hovered ? 0.4 : 0.26)
                    implicitWidth: root.denseMode ? 52 : 58
                    implicitHeight: root.denseMode ? 20 : 22
                    scale: endArea.pressed && root.canTerminate ? 0.97 : 1.0
                    transformOrigin: Item.Center
                    Accessible.role: Accessible.Button
                    Accessible.name: root.sessionName.length > 0
                                     ? "End session " + root.sessionName
                                     : "End session"
                    Accessible.onPressAction: {
                        if (root.canTerminate) {
                            root.terminateRequested(root.sessionId)
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
                        id: endArea

                        anchors.fill: parent
                        enabled: root.canTerminate
                        hoverEnabled: true
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        onEntered: {
                            endButton.hovered = true
                            if (root.hoverHost && root.hoverHost.queueHoverHint) {
                                root.hoverHost.queueHoverHint("Kill this tmux session and mark it exited in Hydra.",
                                                              endButton)
                            }
                        }
                        onExited: {
                            endButton.hovered = false
                            if (root.hoverHost && root.hoverHost.clearHoverHint) {
                                root.hoverHost.clearHoverHint(endButton)
                            }
                        }
                        onClicked: root.terminateRequested(root.sessionId)
                    }

                    Text {
                        anchors.centerIn: parent
                        text: root.denseMode ? "[END]" : "[END]"
                        color: HydraTheme.danger
                        font.family: HydraTheme.monoFamily
                        font.pixelSize: root.denseMode ? 8 : 9
                        font.bold: true
                    }
                }
            }

            Text {
                visible: !root.compactMode && !root.denseMode
                text: root.updatedAtText
                color: HydraTheme.textOnLightSoft
                font.pixelSize: 9
                font.family: HydraTheme.monoFamily
                horizontalAlignment: Text.AlignRight
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: HydraTheme.space4
            }
        }
    }
}
