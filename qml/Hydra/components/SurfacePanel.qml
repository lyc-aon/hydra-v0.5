pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import "../styles"

Rectangle {
    id: root

    property color panelColor: HydraTheme.railPanel
    property color panelBorderColor: HydraTheme.borderDark
    property int panelRadius: HydraTheme.radius10
    property int contentMargin: HydraTheme.space10
    property int contentSpacing: HydraTheme.space8
    default property alias contentData: contentLayout.data

    color: panelColor
    border.width: 1
    border.color: panelBorderColor
    radius: panelRadius
    implicitHeight: contentLayout.implicitHeight + (contentMargin * 2)

    ColumnLayout {
        id: contentLayout

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: root.contentMargin
        spacing: root.contentSpacing
    }
}
