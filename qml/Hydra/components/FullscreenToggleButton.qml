pragma ComponentBehavior: Bound
import QtQuick 6.5
import "../styles"

Rectangle {
    id: root

    property bool activeState: false
    property bool hovered: false
    property var hoverHost: null
    property string accessibleLabel: root.activeState
                                     ? "Exit borderless fullscreen"
                                     : "Enter borderless fullscreen"
    property string hoverText: root.activeState
                               ? "Return Hydra to its windowed shell. Shortcut: F11."
                               : "Enter borderless fullscreen mode. Shortcut: F11."

    signal triggered()

    implicitWidth: 48
    implicitHeight: 24
    radius: HydraTheme.radius4
    color: root.hovered
           ? HydraTheme.withAlpha(HydraTheme.themeControlFill, 0.22)
           : HydraTheme.withAlpha(HydraTheme.themeControlFill, 0.12)
    border.width: 1
    border.color: root.hovered
                  ? HydraTheme.withAlpha(HydraTheme.themeControlTone, 0.72)
                  : HydraTheme.withAlpha(HydraTheme.themeControlTone, 0.34)
    scale: area.pressed ? 0.97 : (root.hovered ? 1.02 : 1.0)
    transformOrigin: Item.Center
    Accessible.role: Accessible.Button
    Accessible.name: root.accessibleLabel
    Accessible.onPressAction: root.triggered()

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
        onContainsMouseChanged: if (containsMouse) HydraSounds.playHover()
        cursorShape: Qt.PointingHandCursor
        onEntered: {
            root.hovered = true
            if (root.hoverHost && root.hoverHost.queueHoverHint) {
                root.hoverHost.queueHoverHint(root.hoverText, root)
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

    Canvas {
        width: 12
        height: 12
        anchors.left: parent.left
        anchors.leftMargin: 6
        anchors.verticalCenter: parent.verticalCenter
        antialiasing: false

        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.lineWidth = 1
            ctx.strokeStyle = HydraTheme.withAlpha(
                        root.hovered ? HydraTheme.themeControlTone : HydraTheme.themeControlText,
                        root.activeState ? 0.96 : 0.82)

            const inset = root.activeState ? 1.5 : 0
            const left = inset
            const top = inset
            const right = width - inset
            const bottom = height - inset
            const span = 4

            ctx.beginPath()
            ctx.moveTo(left, top + span)
            ctx.lineTo(left, top)
            ctx.lineTo(left + span, top)

            ctx.moveTo(right - span, top)
            ctx.lineTo(right, top)
            ctx.lineTo(right, top + span)

            ctx.moveTo(left, bottom - span)
            ctx.lineTo(left, bottom)
            ctx.lineTo(left + span, bottom)

            ctx.moveTo(right - span, bottom)
            ctx.lineTo(right, bottom)
            ctx.lineTo(right, bottom - span)
            ctx.stroke()
        }

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
    }

    Text {
        anchors.right: parent.right
        anchors.rightMargin: 6
        anchors.verticalCenter: parent.verticalCenter
        text: "F11"
        color: root.hovered
               ? HydraTheme.themeControlTone
               : HydraTheme.withAlpha(HydraTheme.themeControlText, 0.86)
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        font.bold: true
        font.letterSpacing: 0.4
    }
}
