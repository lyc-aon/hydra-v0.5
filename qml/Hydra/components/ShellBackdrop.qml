pragma ComponentBehavior: Bound
import QtQuick 6.5
import "../styles"

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: HydraTheme.shellBg

        gradient: Gradient {
            GradientStop { position: 0.0; color: HydraTheme.shellDepth }
            GradientStop { position: 0.28; color: HydraTheme.shellBg }
            GradientStop { position: 1.0; color: HydraTheme.shellDepthSoft }
        }
    }

    Repeater {
        model: Math.ceil(root.width / 160)

        Rectangle {
            required property int index

            x: index * 160
            y: 0
            width: 1
            height: root.height
            color: HydraTheme.withAlpha(HydraTheme.gridLine, 0.22)
        }
    }

    Repeater {
        model: Math.ceil(root.height / 120)

        Rectangle {
            required property int index

            x: 0
            y: index * 120
            width: root.width
            height: 1
            color: HydraTheme.withAlpha(HydraTheme.gridLine, 0.14)
        }
    }

    Repeater {
        model: Math.ceil(root.height / 112)

        Item {
            id: segmentRow
            required property int index
            readonly property int rowIndex: index

            y: index * 112 + 42
            width: root.width
            height: 18

            Repeater {
                model: Math.ceil(root.width / 172)

                Rectangle {
                    required property int index

                    x: (segmentRow.rowIndex % 2 === 0 ? 18 : 68) + index * 172
                    y: 0
                    width: 56
                    height: 1
                    color: HydraTheme.withAlpha(HydraTheme.accentSteel, 0.12)
                }
            }

            Repeater {
                model: Math.ceil(root.width / 172)

                Rectangle {
                    required property int index

                    x: (segmentRow.rowIndex % 2 === 0 ? 46 : 96) + index * 172
                    y: 10
                    width: 28
                    height: 1
                    color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.1)
                }
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 1
        color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.72)
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        width: Math.min(240, parent.width * 0.2)
        height: 2
        color: HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.42)
    }
}
