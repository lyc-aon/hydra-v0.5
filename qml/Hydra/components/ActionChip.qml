pragma ComponentBehavior: Bound
import QtQuick 6.5
import "../styles"

Rectangle {
    id: root

    property string text: ""
    property color toneColor: HydraTheme.accentSteelBright
    property real fillOpacity: 0.05
    property real borderOpacity: 0.22
    property int textPixelSize: 10
    property int horizontalPadding: HydraTheme.space8
    property int verticalPadding: HydraTheme.space4
    property int minWidth: 0
    property bool active: false
    property string toolTipText: ""
    property string accessibleLabel: text
    property var hoverHost: null

    signal clicked()

    readonly property bool ogSteamTheme: HydraTheme.currentThemeId === "og_steam"
    readonly property bool isHovered: mouseArea.containsMouse
    readonly property real effectiveFillOpacity: {
        var base = root.active ? 0.18
                               : (root.isHovered ? 0.12 : root.fillOpacity)
        return root.ogSteamTheme ? Math.min(0.3, base + 0.04) : base
    }
    readonly property real effectiveBorderOpacity: {
        var base = root.isHovered ? 0.38 : root.borderOpacity
        return root.ogSteamTheme ? Math.min(0.62, base + 0.1) : base
    }

    implicitWidth: Math.max(minWidth, label.implicitWidth + (horizontalPadding * 2))
    implicitHeight: label.implicitHeight + (verticalPadding * 2)
    radius: HydraTheme.radius4
    color: HydraTheme.withAlpha(root.toneColor, root.effectiveFillOpacity)
    border.width: 1
    border.color: HydraTheme.withAlpha(root.toneColor, root.effectiveBorderOpacity)
    scale: mouseArea.pressed ? 0.97 : 1.0
    transformOrigin: Item.Center
    activeFocusOnTab: true

    Accessible.role: Accessible.Button
    Accessible.ignored: !root.visible || root.width <= 0 || root.height <= 0
    Accessible.name: root.accessibleLabel
    Accessible.onPressAction: root.clicked()

    Behavior on scale {
        NumberAnimation {
            duration: HydraTheme.motionFast
            easing.type: Easing.OutCubic
        }
    }

    Behavior on color {
        ColorAnimation { duration: HydraTheme.motionFast }
    }

    Behavior on border.color {
        ColorAnimation { duration: HydraTheme.motionFast }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        hoverEnabled: true
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: { HydraSounds.playClick(); root.clicked() }
        onContainsMouseChanged: {
            if (containsMouse) HydraSounds.playHover()
            if (!root.hoverHost || root.toolTipText.length === 0) {
                return
            }
            if (containsMouse && root.hoverHost.queueHoverHint) {
                root.hoverHost.queueHoverHint(root.toolTipText, root)
            } else if (!containsMouse && root.hoverHost.clearHoverHint) {
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
        color: root.enabled ? root.toneColor : HydraTheme.textOnDarkMuted
        font.family: HydraTheme.monoFamily
        font.pixelSize: root.textPixelSize
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        clip: true
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
            root.clicked()
            event.accepted = true
        }
    }
}
