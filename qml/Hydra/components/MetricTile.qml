import QtQuick 6.5
import QtQuick.Layouts 6.5
import "../styles"

Rectangle {
    id: root

    property string label: ""
    property string value: ""
    property bool monospaceValue: false
    property bool valueMonospace: monospaceValue
    property color valueColor: HydraTheme.textOnLight
    property string baseFontFamily: HydraTheme.bodyFamily
    property bool compactMode: false

    implicitHeight: root.compactMode ? 46 : 54
    radius: HydraTheme.radius6
    color: HydraTheme.boardPanelMuted
    border.width: 1
    border.color: HydraTheme.borderDark

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 2
        color: HydraTheme.withAlpha(root.valueColor, 0.74)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: root.compactMode ? HydraTheme.space8 : HydraTheme.space10
        anchors.rightMargin: root.compactMode ? HydraTheme.space8 : HydraTheme.space10
        anchors.topMargin: root.compactMode ? HydraTheme.space8 : HydraTheme.space10
        anchors.bottomMargin: root.compactMode ? HydraTheme.space8 : HydraTheme.space8
        spacing: HydraTheme.space2

        Text {
            text: "[" + root.label + "]"
            color: HydraTheme.textOnLightSoft
            font.family: HydraTheme.monoFamily
            font.pixelSize: root.compactMode ? 8 : 9
            font.bold: true
            font.letterSpacing: 1.0
        }

        Text {
            text: root.value
            color: root.valueColor
            font.pixelSize: root.compactMode ? 13 : 15
            font.bold: true
            font.family: root.valueMonospace ? HydraTheme.monoFamily : root.baseFontFamily
            elide: Text.ElideRight
            Layout.fillWidth: true
            Layout.fillHeight: true
            verticalAlignment: Text.AlignBottom
        }
    }
}
