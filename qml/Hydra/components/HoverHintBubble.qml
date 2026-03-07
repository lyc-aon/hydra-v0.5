pragma ComponentBehavior: Bound
import QtQuick 6.5
import "../styles"

Item {
    id: root

    property string text: ""
    property real bubbleX: 0
    property real bubbleY: 0
    property int bubbleWidth: 248

    visible: text.length > 0

    Rectangle {
        x: root.bubbleX
        y: root.bubbleY
        width: root.bubbleWidth
        height: hintText.implicitHeight + (HydraTheme.space10 * 2)
        radius: HydraTheme.radius6
        color: HydraTheme.boardPanel
        border.width: 1
        border.color: HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.34)
        opacity: root.visible ? 1.0 : 0.0
        scale: root.visible ? 1.0 : 0.97
        clip: true

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
            color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.54)
        }

        Text {
            id: hintText

            anchors.fill: parent
            anchors.margins: HydraTheme.space10
            text: root.text
            color: HydraTheme.textOnDark
            font.family: HydraTheme.bodyFamily
            font.pixelSize: 11
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
        }
    }
}
