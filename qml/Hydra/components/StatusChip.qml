pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import "../styles"

Rectangle {
    id: root

    property string text: ""
    property color toneColor: HydraTheme.accentSteelBright
    property color textColor: toneColor
    property real fillOpacity: 0.08
    property real borderOpacity: 0.28
    property int minWidth: 0
    property int horizontalPadding: HydraTheme.space8
    property int verticalPadding: HydraTheme.space4
    property int textPixelSize: 8
    property string textFamily: HydraTheme.monoFamily
    property real textLetterSpacing: 0.0
    property bool hovered: false
    property string toolTipText: ""
    property QtObject hoverHost: null

    implicitWidth: Math.max(minWidth, label.implicitWidth + (horizontalPadding * 2))
    implicitHeight: label.implicitHeight + (verticalPadding * 2)
    radius: HydraTheme.radius4
    color: HydraTheme.withAlpha(toneColor, fillOpacity)
    border.width: 1
    border.color: HydraTheme.withAlpha(toneColor, borderOpacity)
    HoverHandler {
        enabled: root.toolTipText.length > 0
        onHoveredChanged: {
            root.hovered = hovered
            if (!root.hoverHost) {
                return
            }
            if (hovered && root.hoverHost.queueHoverHint) {
                root.hoverHost.queueHoverHint(root.toolTipText, root)
            } else if (!hovered && root.hoverHost.clearHoverHint) {
                root.hoverHost.clearHoverHint(root)
            }
        }
    }

    Text {
        id: label

        anchors.centerIn: parent
        text: root.text
        color: root.textColor
        font.family: root.textFamily
        font.pixelSize: root.textPixelSize
        font.bold: true
        font.letterSpacing: root.textLetterSpacing
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
