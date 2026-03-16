pragma ComponentBehavior: Bound
import QtQuick 6.5
import Hydra.Backend 1.0
import "../styles"

Item {
    id: root

    property AppState appState: null
    property bool tightMode: false
    property real reveal: 1.0
    property var sectionAnchors: []

    readonly property real primaryLaneX: Math.round(root.width * 0.1)
    readonly property real secondaryLaneX: Math.round(root.width * 0.19)
    readonly property real tertiaryLaneX: Math.round(root.width * 0.31)
    readonly property real returnLaneX: Math.round(root.width * 0.72)
    readonly property color signalColor: {
        if (!root.appState) {
            return HydraTheme.panelRule
        }
        switch (root.appState.lastActivityKind) {
        case "launch":
            return HydraTheme.accentReady
        case "worktree":
            return HydraTheme.accentBronze
        case "terminate":
            return HydraTheme.danger
        case "refresh":
        default:
            return HydraTheme.panelRule
        }
    }
    readonly property int inventoryMarkerCount: Math.max(3,
                                                         Math.min(14,
                                                                  root.appState
                                                                  ? (root.appState.repoCount
                                                                     + root.appState.worktreeCount
                                                                     + root.appState.sessionCount)
                                                                  : 0))

    opacity: reveal
    clip: true

    readonly property bool evaTheme: HydraTheme.currentThemeId === "eva"

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: HydraTheme.withAlpha(HydraTheme.railFieldTint, root.evaTheme ? 0.06 : 0.22) }
            GradientStop { position: 0.3; color: HydraTheme.withAlpha(HydraTheme.panelLeadSoft, root.evaTheme ? 0.02 : 0.08) }
            GradientStop { position: 0.75; color: "transparent" }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }

    Repeater {
        model: [
            { "x": root.primaryLaneX, "color": HydraTheme.railSignalPrimary, "opacity": 0.28 },
            { "x": root.secondaryLaneX, "color": HydraTheme.railSignalSecondary, "opacity": 0.22 },
            { "x": root.tertiaryLaneX, "color": HydraTheme.railSignalTertiary, "opacity": 0.14 },
            { "x": root.returnLaneX, "color": HydraTheme.panelRule, "opacity": 0.12 }
        ]

        Rectangle {
            required property var modelData

            x: modelData.x
            y: 0
            width: 1
            height: root.height
            color: HydraTheme.withAlpha(modelData.color, modelData.opacity)
        }
    }

    Repeater {
        model: Math.max(0, Math.ceil(Math.max(0, root.height) / 48))

        Item {
            id: lanePacket

            required property int index

            x: 0
            y: lanePacket.index * 48 + 8
            width: root.width
            height: 14

            Rectangle {
                x: root.primaryLaneX - 2
                y: 4
                width: 5
                height: 5
                radius: 1
                color: HydraTheme.withAlpha(lanePacket.index % 3 === 0 ? HydraTheme.railPacketColor : HydraTheme.railSignalSecondary,
                                            lanePacket.index % 2 === 0 ? 0.34 : 0.18)
            }

            Rectangle {
                x: root.secondaryLaneX - 1
                y: 6
                width: lanePacket.index % 2 === 0 ? 18 : 28
                height: 1
                color: HydraTheme.withAlpha(HydraTheme.ambientTrace, 0.18)
            }

            Rectangle {
                visible: !root.tightMode && lanePacket.index % 3 === 1
                x: root.secondaryLaneX + 22
                y: 6
                width: Math.max(20, root.returnLaneX - root.secondaryLaneX - 44)
                height: 1
                color: HydraTheme.withAlpha(HydraTheme.ambientTraceBright, 0.09)
            }
        }
    }

    Repeater {
        model: root.inventoryMarkerCount

        Rectangle {
            required property int index

            x: (index % 2 === 0 ? root.primaryLaneX : root.secondaryLaneX) - 3
            y: Math.round(((index + 1) * root.height) / (root.inventoryMarkerCount + 1)) - 3
            width: index % 4 === 0 ? 7 : 5
            height: index % 4 === 0 ? 7 : 5
            radius: 1
            color: HydraTheme.withAlpha(index % 3 === 0 ? root.signalColor : HydraTheme.railSignalPrimary,
                                        index % 2 === 0 ? 0.26 : 0.14)
        }
    }

    Repeater {
        model: root.sectionAnchors

        Item {
            required property var modelData

            x: 0
            y: Math.max(0, Math.min(root.height - 20, Number(modelData) - 8))
            width: root.width
            height: 20

            Rectangle {
                x: root.primaryLaneX - 4
                y: 9
                width: 9
                height: 1
                color: HydraTheme.withAlpha(root.signalColor, 0.34)
            }

            Rectangle {
                x: root.primaryLaneX + 6
                y: 9
                width: Math.max(24, root.secondaryLaneX - root.primaryLaneX + 18)
                height: 1
                color: HydraTheme.withAlpha(HydraTheme.railPulseColor, 0.16)
            }

            Rectangle {
                x: root.secondaryLaneX + 17
                y: 6
                width: 12
                height: 6
                radius: 1
                color: HydraTheme.withAlpha(HydraTheme.panelLead, 0.14)
            }

            Rectangle {
                visible: !root.tightMode
                x: root.secondaryLaneX + 33
                y: 9
                width: Math.max(18, root.returnLaneX - root.secondaryLaneX - 38)
                height: 1
                color: HydraTheme.withAlpha(HydraTheme.panelRule, 0.08)
            }
        }
    }
}
