pragma ComponentBehavior: Bound
import QtQuick 6.5
import "../styles"

Rectangle {
    id: root

    property string text: ""
    property string shortcutHint: ""
    property string accessibleLabel: text
    property string toolTipText: ""
    property color toneColor: HydraTheme.accentSteelBright
    property bool active: false
    property bool selected: false
    property bool enabledState: true
    property var hoverHost: null
    property bool hovered: false
    property int minWidth: 96
    property int textPixelSize: 10
    property string textFamily: HydraTheme.monoFamily
    property real textLetterSpacing: 0.8

    signal triggered()

    readonly property bool ogSteamTheme: HydraTheme.currentThemeId === "og_steam"
    readonly property bool evaTheme: HydraTheme.currentThemeId === "eva"
    readonly property int contentWidth: labelText.implicitWidth
                                        + (shortcutText.visible ? (HydraTheme.space6 + shortcutText.implicitWidth) : 0)

    implicitWidth: Math.max(minWidth, contentWidth + (HydraTheme.space8 * 2))
    implicitHeight: 32
    activeFocusOnTab: enabledState
    radius: HydraTheme.radius4
    transformOrigin: Item.Center
    scale: actionArea.pressed && enabledState ? 0.97 : 1.0
    opacity: enabledState || selected ? 1.0 : 0.58

    color: {
        if (selected) {
            return HydraTheme.withAlpha(toneColor,
                ogSteamTheme ? 0.24 : (evaTheme ? 0.16 : 0.18))
        }
        if (active) {
            return HydraTheme.withAlpha(toneColor, 0.16)
        }
        if (hovered) {
            return HydraTheme.withAlpha(toneColor, 0.12)
        }
        return HydraTheme.withAlpha(toneColor, 0.03)
    }

    border.width: 1
    border.color: {
        if (selected) {
            return HydraTheme.withAlpha(toneColor,
                activeFocus ? 0.96 : (ogSteamTheme ? 0.84 : 0.72))
        }
        if (active) {
            return HydraTheme.withAlpha(toneColor, 0.6)
        }
        if (!enabledState) {
            return HydraTheme.withAlpha(HydraTheme.danger,
                ogSteamTheme ? 0.56 : 0.42)
        }
        return (hovered || activeFocus) ? HydraTheme.borderFocus : HydraTheme.borderDark
    }

    Accessible.role: Accessible.Button
    Accessible.name: root.accessibleLabel
    Accessible.onPressAction: {
        if (root.enabledState) {
            root.triggered()
        }
    }

    Keys.onPressed: event => {
        if (!root.enabledState) {
            return
        }
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
            root.triggered()
            event.accepted = true
        }
    }

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
        id: actionArea

        anchors.fill: parent
        enabled: root.enabledState
        hoverEnabled: true
        onContainsMouseChanged: if (containsMouse) HydraSounds.playHover()
        cursorShape: root.enabledState ? Qt.PointingHandCursor : Qt.ArrowCursor
        onEntered: {
            root.hovered = true
            if (root.hoverHost && root.hoverHost.queueHoverHint && root.toolTipText.length > 0) {
                root.hoverHost.queueHoverHint(root.toolTipText, root)
            }
        }
        onExited: {
            root.hovered = false
            if (root.hoverHost && root.hoverHost.clearHoverHint) {
                root.hoverHost.clearHoverHint(root)
            }
        }
        onClicked: { HydraSounds.playClick(); root.triggered() }
    }

    Row {
        anchors.fill: parent
        anchors.leftMargin: HydraTheme.space8
        anchors.rightMargin: HydraTheme.space8
        anchors.topMargin: HydraTheme.space4
        anchors.bottomMargin: HydraTheme.space4
        spacing: HydraTheme.space6

        Text {
            id: labelText

            anchors.verticalCenter: parent.verticalCenter
            text: root.text
            color: {
                if (!root.enabledState) {
                    return HydraTheme.textOnDarkMuted
                }
                if (root.selected || root.active || root.hovered) {
                    return root.toneColor
                }
                return HydraTheme.textOnDark
            }
            font.family: root.textFamily
            font.pixelSize: root.textPixelSize
            font.bold: true
            font.letterSpacing: root.textLetterSpacing
        }

        Text {
            id: shortcutText

            anchors.verticalCenter: parent.verticalCenter
            visible: root.shortcutHint.length > 0
            text: root.shortcutHint
            color: root.enabledState
                   ? HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.88)
                   : HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.72)
            font.family: HydraTheme.monoFamily
            font.pixelSize: 10
            font.bold: true
            font.letterSpacing: 0.4
        }
    }
}
