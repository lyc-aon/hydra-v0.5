pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import "../styles"

Item {
    id: root

    property string title: ""
    property color titleColor: HydraTheme.accentBronze
    property int titlePixelSize: 13
    property real titleLetterSpacing: 1.3
    default property alias accessoryData: accessoryHost.data

    implicitWidth: headerRow.implicitWidth
    implicitHeight: headerRow.implicitHeight

    RowLayout {
        id: headerRow

        anchors.fill: parent
        spacing: HydraTheme.space8

        Text {
            text: root.title
            color: root.titleColor
            font.family: HydraTheme.displayFamily
            font.pixelSize: root.titlePixelSize
            font.bold: true
            font.letterSpacing: root.titleLetterSpacing
            Layout.fillWidth: true
        }

        RowLayout {
            id: accessoryHost

            spacing: HydraTheme.space8
        }
    }
}
