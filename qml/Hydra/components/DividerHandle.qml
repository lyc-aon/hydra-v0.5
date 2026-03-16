pragma ComponentBehavior: Bound
import QtQuick 6.5
import "../styles"

Item {
    id: root

    property bool tightMode: false
    property bool sidebarCollapsed: false
    property int panelInset: HydraTheme.space12
    property var hoverHost: null
    property bool dragActive: dragHandler.active

    signal toggleRequested()
    signal dragStarted()
    signal dragMoved(real deltaX)
    signal dragFinished()

    width: tightMode ? 14 : 16
    height: tightMode ? 50 : 54

    Rectangle {
        id: button

        property bool hovered: false

        anchors.fill: parent
        radius: root.tightMode ? HydraTheme.radius8 : HydraTheme.radius10
        color: tapHandler.pressed || dragHandler.active
               ? HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.12)
               : (hovered
                  ? HydraTheme.withAlpha(HydraTheme.accentSteel, 0.08)
                  : HydraTheme.withAlpha(HydraTheme.railPanelStrong, 0.78))
        border.width: 1
        border.color: hovered || tapHandler.pressed || dragHandler.active
                      ? HydraTheme.withAlpha(HydraTheme.borderFocus, 0.72)
                      : HydraTheme.withAlpha(HydraTheme.borderDark, 0.38)
        scale: tapHandler.pressed || dragHandler.active ? 0.97 : 1.0
        transformOrigin: Item.Center
        Accessible.role: Accessible.Button
        Accessible.name: "Toggle navigation rail"
        Accessible.description: "Click to show or hide the navigation rail. Drag to resize it."
        Accessible.onPressAction: root.toggleRequested()
        Behavior on color {
            ColorAnimation { duration: HydraTheme.motionFast }
        }

        Behavior on scale {
            NumberAnimation {
                duration: HydraTheme.motionFast
                easing.type: Easing.OutCubic
            }
        }

        Column {
            anchors.centerIn: parent
            spacing: HydraTheme.space6

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: root.sidebarCollapsed ? ">>" : "<<"
                color: button.hovered || tapHandler.pressed || dragHandler.active
                       ? HydraTheme.accentBronze
                       : HydraTheme.textOnDarkMuted
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                font.bold: true
            }

            Column {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 3

                Repeater {
                    model: 2

                    Rectangle {
                        width: root.tightMode ? 5 : 6
                        height: 1
                        radius: 1
                        color: HydraTheme.withAlpha(HydraTheme.accentSteel, 0.66)
                    }
                }
            }
        }

        HoverHandler {
            cursorShape: Qt.SizeHorCursor
            onHoveredChanged: {
                button.hovered = hovered
                if (!root.hoverHost) {
                    return
                }
                if (hovered && root.hoverHost.queueHoverHint) {
                    root.hoverHost.queueHoverHint("Click to hide or show the rail. Drag to resize it.", button)
                } else if (!hovered && root.hoverHost.clearHoverHint) {
                    root.hoverHost.clearHoverHint(button)
                }
            }
        }

        TapHandler {
            id: tapHandler
            onTapped: root.toggleRequested()
        }

        DragHandler {
            id: dragHandler

            target: null
            xAxis.enabled: true
            yAxis.enabled: false
            onActiveChanged: {
                if (active) {
                    root.dragStarted()
                    return
                }

                root.dragFinished()
            }
            onTranslationChanged: {
                if (active) {
                    root.dragMoved(translation.x)
                }
            }
        }
    }
}
