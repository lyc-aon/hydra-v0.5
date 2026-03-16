pragma ComponentBehavior: Bound
import QtQuick 6.5
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
    property int textPixelSize: 10
    property string textFamily: HydraTheme.monoFamily
    property real textLetterSpacing: 0.0
    property bool hovered: false
    property string toolTipText: ""
    property string accessibleLabel: text
    property var hoverHost: null
    readonly property bool ogSteamTheme: HydraTheme.currentThemeId === "og_steam"
    readonly property real effectiveFillOpacity: ogSteamTheme
                                                 ? Math.min(0.3, fillOpacity + 0.04)
                                                 : fillOpacity
    readonly property real effectiveBorderOpacity: ogSteamTheme
                                                   ? Math.min(0.62, borderOpacity + 0.1)
                                                   : borderOpacity

    implicitWidth: Math.max(minWidth, label.implicitWidth + (horizontalPadding * 2))
    implicitHeight: label.implicitHeight + (verticalPadding * 2)
    radius: HydraTheme.radius4
    color: HydraTheme.withAlpha(toneColor, effectiveFillOpacity)
    border.width: 1
    border.color: HydraTheme.withAlpha(toneColor, effectiveBorderOpacity)
    Accessible.role: Accessible.StaticText
    Accessible.ignored: !root.visible || root.width <= 0 || root.height <= 0
    Accessible.name: accessibleLabel
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

        anchors.fill: parent
        anchors.leftMargin: root.horizontalPadding
        anchors.rightMargin: root.horizontalPadding
        anchors.topMargin: root.verticalPadding
        anchors.bottomMargin: root.verticalPadding
        text: root.text
        color: root.textColor
        font.family: root.textFamily
        font.pixelSize: root.textPixelSize
        fontSizeMode: Text.HorizontalFit
        minimumPixelSize: 6
        font.bold: true
        font.letterSpacing: root.textLetterSpacing
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        clip: true
    }
}
