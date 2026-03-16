pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import Hydra.Backend 1.0
import "../styles"

Rectangle {
    id: root

    required property ThemeState themeState
    property var hoverHost: null
    property bool denseMode: false
    property bool tightMode: false
    property bool hovered: false

    implicitWidth: Math.max(root.tightMode ? 94 : (root.denseMode ? 106 : 120),
                            themeLabel.implicitWidth + 80)
    implicitHeight: root.denseMode ? 22 : 24
    radius: HydraTheme.radius4
    color: area.pressed
           ? HydraTheme.withAlpha(HydraTheme.themeControlFill, 0.3)
           : (root.hovered
              ? HydraTheme.withAlpha(HydraTheme.themeControlFill, 0.24)
              : HydraTheme.withAlpha(HydraTheme.themeControlFill, 0.16))
    border.width: 1
    border.color: root.hovered
                  ? HydraTheme.withAlpha(HydraTheme.themeControlTone, 0.62)
                  : HydraTheme.withAlpha(HydraTheme.themeControlTone, 0.34)
    clip: true
    scale: area.pressed ? 0.98 : 1.0
    transformOrigin: Item.Center
    Accessible.role: Accessible.Button
    Accessible.name: "Theme cycle " + root.themeState.currentThemeLabel
    Accessible.description: "Cycle the active Hydra color theme."
    Accessible.onPressAction: root.themeState.cycleTheme()

    Behavior on color {
        ColorAnimation { duration: HydraTheme.motionFast }
    }

    Behavior on scale {
        NumberAnimation {
            duration: HydraTheme.motionFast
            easing.type: Easing.OutCubic
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 2
        color: HydraTheme.withAlpha(HydraTheme.themeControlTone, 0.78)
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 1
        color: HydraTheme.withAlpha(HydraTheme.themeControlTone, 0.18)
    }

    Rectangle {
        id: packet

        width: 16
        height: 1
        y: parent.height - 2
        x: root.width - width - 6
        color: HydraTheme.withAlpha(HydraTheme.themeControlTone, 0.4)
        visible: HydraTheme.ambientEnabled && (root.hovered || root.activeFocus)
    }

    MouseArea {
        id: area

        anchors.fill: parent
        hoverEnabled: true
        onContainsMouseChanged: if (containsMouse) HydraSounds.playHover()
        cursorShape: Qt.PointingHandCursor
        onEntered: {
            root.hovered = true
            if (root.hoverHost && root.hoverHost.queueHoverHint) {
                root.hoverHost.queueHoverHint("Current theme: " + root.themeState.currentThemeLabel + ". Click to cycle the available color themes.",
                                             root)
            }
        }
        onExited: {
            root.hovered = false
            if (root.hoverHost && root.hoverHost.clearHoverHint) {
                root.hoverHost.clearHoverHint(root)
            }
        }
        onClicked: { HydraSounds.playClick(); root.themeState.cycleTheme() }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: HydraTheme.space8
        anchors.rightMargin: HydraTheme.space8
        spacing: HydraTheme.space6

        Text {
            text: "THEME"
            color: HydraTheme.themeControlTone
            font.family: HydraTheme.monoFamily
            font.pixelSize: 10
            font.bold: true
            font.letterSpacing: 0.8
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: HydraTheme.withAlpha(HydraTheme.themeControlTone, 0.2)
        }

        Text {
            id: themeLabel

            text: root.themeState.currentThemeLabel
            color: HydraTheme.themeControlText
            font.family: HydraTheme.displayFamily
            font.pixelSize: root.denseMode ? 10 : 11
            font.bold: true
            font.letterSpacing: 0.8
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        Text {
            text: ">"
            color: HydraTheme.withAlpha(HydraTheme.themeControlText, 0.82)
            font.family: HydraTheme.monoFamily
            font.pixelSize: 10
            font.bold: true
        }
    }
}
