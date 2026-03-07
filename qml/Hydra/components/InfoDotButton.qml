import QtQuick 6.5
import QtQuick.Controls 6.5
import "../styles"

Rectangle {
    id: root

    property string topicId: ""
    property string briefText: ""
    property string accessibleLabel: "Show help"
    property bool hovered: false
    property QtObject hoverHost: null

    signal helpRequested(string topicId, Item source)

    implicitWidth: 18
    implicitHeight: 18
    radius: width / 2
    color: area.pressed
           ? HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.22)
           : (root.hovered
              ? HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.14)
              : HydraTheme.withAlpha(HydraTheme.railPanelStrong, 0.92))
    border.width: 1
    border.color: root.hovered || area.pressed ? HydraTheme.borderFocus : HydraTheme.borderDark
    scale: area.pressed ? 0.96 : (root.hovered ? 1.04 : 1.0)
    transformOrigin: Item.Center
    Accessible.role: Accessible.Button
    Accessible.name: root.accessibleLabel
    Accessible.description: root.briefText
    Accessible.onPressAction: root.helpRequested(root.topicId, root)

    Behavior on color {
        ColorAnimation { duration: HydraTheme.motionFast }
    }

    Behavior on scale {
        NumberAnimation {
            duration: HydraTheme.motionFast
            easing.type: Easing.OutCubic
        }
    }

    MouseArea {
        id: area

        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onEntered: {
            root.hovered = true
            if (root.hoverHost && root.hoverHost.queueHoverHint) {
                root.hoverHost.queueHoverHint(root.briefText, root)
            }
        }
        onExited: {
            root.hovered = false
            if (root.hoverHost && root.hoverHost.clearHoverHint) {
                root.hoverHost.clearHoverHint(root)
            }
        }
        onClicked: {
            if (root.hoverHost && root.hoverHost.clearHoverHint) {
                root.hoverHost.clearHoverHint(root)
            }
            root.helpRequested(root.topicId, root)
        }
    }

    Text {
        anchors.centerIn: parent
        text: "i"
        color: root.hovered || area.pressed ? HydraTheme.accentPhosphor : HydraTheme.textOnDarkMuted
        font.family: HydraTheme.displayFamily
        font.pixelSize: 11
        font.bold: true
    }
}
