pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import Hydra.Backend 1.0
import "../styles"

FocusScope {
    id: root

    required property AppState appState
    required property bool collapsed
    required property var stateDistribution
    signal collapseToggled()
    signal sessionClicked(string sessionId)

    function sessionNodeAt(index: int): var {
        return sessionNodes.itemAtIndex(index)
    }

    function flashSession(sessionId: string) {
        for (var i = 0; i < sessionNodes.count; ++i) {
            var item = root.sessionNodeAt(i)
            if (item && item["sessionId"] === sessionId && item["flashRoute"]) {
                item["flashRoute"]()
                break
            }
        }
    }

    readonly property int expandedHeight: 104
    readonly property bool focusWithin: root.activeFocus
    property real reveal: root.collapsed ? 0.0 : 1.0

    implicitHeight: headerRow.implicitHeight + Math.round(expandedHeight * reveal)

    Behavior on reveal {
        NumberAnimation { duration: HydraTheme.motionSlow; easing.type: Easing.InOutCubic }
    }

    Accessible.role: Accessible.Grouping
    Accessible.name: "Session orbit strip"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header
        RowLayout {
            id: headerRow
            Layout.fillWidth: true
            Layout.minimumHeight: 22
            spacing: HydraTheme.space8

            Text {
                text: "ORBIT MATRIX"
                color: HydraTheme.textOnDarkMuted
                font.family: HydraTheme.displayFamily
                font.pixelSize: 11
                font.bold: true
                font.letterSpacing: 1.4
            }

            Item { Layout.fillWidth: true }

            ActionChip {
                text: root.collapsed ? "[" + "\u25BC" + "]" : "[" + "\u25B2" + "]"
                toneColor: HydraTheme.accentSteelBright
                accessibleLabel: root.collapsed ? "Expand orbit strip" : "Collapse orbit strip"
                onClicked: root.collapseToggled()
            }
        }

        // Content
        Item {
            Layout.fillWidth: true
            implicitHeight: Math.round(root.expandedHeight * root.reveal)
            clip: true
            opacity: root.reveal
            visible: root.reveal > 0.001

            Rectangle {
                anchors.fill: parent
                anchors.topMargin: HydraTheme.space4
                radius: HydraTheme.radius10
                gradient: Gradient {
                    GradientStop { position: 0.0; color: HydraTheme.withAlpha(HydraTheme.boardPanel, 0.92) }
                    GradientStop { position: 1.0; color: HydraTheme.withAlpha(HydraTheme.chromeGlass, 0.86) }
                }
                border.width: 1
                border.color: HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.24)

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: HydraTheme.space10
                    spacing: HydraTheme.space10

                    ListView {
                        id: sessionNodes
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        orientation: ListView.Horizontal
                        spacing: HydraTheme.space12
                        clip: true
                        model: root.appState.sessionModel
                        interactive: contentWidth > width

                        delegate: SessionSyncNode {
                            denseMode: sessionNodes.width < 560
                            onSessionSelected: sid => root.sessionClicked(sid)
                        }
                    }

                    ColumnLayout {
                        Layout.alignment: Qt.AlignVCenter
                        spacing: HydraTheme.space6

                        AggregatePulseRing {
                            Layout.alignment: Qt.AlignHCenter
                            stateDistribution: root.stateDistribution
                        }

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: root.appState.sessionCount > 0
                                  ? ("[" + root.appState.sessionCount + " live]")
                                  : "[idle grid]"
                            color: HydraTheme.textOnDarkMuted
                            font.family: HydraTheme.monoFamily
                            font.pixelSize: 9
                            font.letterSpacing: 0.8
                        }
                    }
                }
            }
        }
    }
}
