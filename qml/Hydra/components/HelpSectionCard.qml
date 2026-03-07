pragma ComponentBehavior: Bound
import QtQuick 6.5
import "../styles"

Rectangle {
    id: root

    property string title: ""
    property color accentColor: HydraTheme.accentBronze
    property var lines: []

    function formattedLines() {
        if (!root.lines || root.lines.length === 0) {
            return ""
        }

        return root.lines.map(function(line) {
            return "- " + line
        }).join("\n")
    }

    width: parent ? parent.width : implicitWidth
    implicitHeight: content.implicitHeight + (HydraTheme.space10 * 2)
    radius: HydraTheme.radius8
    color: HydraTheme.withAlpha(HydraTheme.boardPanelMuted, 0.76)
    border.width: 1
    border.color: HydraTheme.withAlpha(HydraTheme.borderDark, 0.88)

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 2
        color: HydraTheme.withAlpha(root.accentColor, 0.78)
    }

    Column {
        id: content

        anchors.fill: parent
        anchors.leftMargin: HydraTheme.space12
        anchors.rightMargin: HydraTheme.space12
        anchors.topMargin: HydraTheme.space10
        anchors.bottomMargin: HydraTheme.space10
        spacing: HydraTheme.space4

        Text {
            text: root.title
            color: HydraTheme.accentBronze
            font.family: HydraTheme.displayFamily
            font.pixelSize: 13
            font.bold: true
            font.letterSpacing: 1.1
        }

        Text {
            text: root.formattedLines()
            color: HydraTheme.textOnDark
            font.family: HydraTheme.bodyFamily
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            width: parent.width
        }
    }
}
