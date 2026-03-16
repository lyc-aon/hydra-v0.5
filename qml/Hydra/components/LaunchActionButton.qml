pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import "../styles"

Rectangle {
    id: root

    required property bool canLaunch
    required property bool armed
    property bool denseMode: false
    property var hoverHost: null
    property string titleText: ""
    property string subtitleText: ""
    property string activeHoverText: ""
    property string inactiveHoverText: ""
    property bool hovered: false

    signal triggered()

    function playArmBurst() {
        if (!root.armed) {
            return
        }
        armBurst.restart()
        armedSweep.restartSweep()
    }

    implicitHeight: root.denseMode ? 40 : 44
    activeFocusOnTab: true
    radius: HydraTheme.radius6
    color: root.armed
           ? (root.hovered
              ? HydraTheme.withAlpha(HydraTheme.accentOrangeStrong, 0.22)
              : HydraTheme.withAlpha(HydraTheme.accentOrangeStrong, 0.12))
           : (root.canLaunch
           ? (root.hovered
              ? HydraTheme.withAlpha(HydraTheme.accentOrangeStrong, 0.16)
              : HydraTheme.withAlpha(HydraTheme.accentOrangeStrong, 0.08))
           : HydraTheme.withAlpha(HydraTheme.accentMuted, 0.42))
    border.width: 1
    border.color: root.armed
                  ? HydraTheme.withAlpha(HydraTheme.accentOrangeStrong,
                                         root.activeFocus ? 1.0 : (root.hovered ? 0.92 : 0.76))
                  : (root.canLaunch
                  ? HydraTheme.withAlpha(HydraTheme.accentOrangeStrong,
                                         root.activeFocus ? 0.88 : (root.hovered ? 0.68 : 0.42))
                  : (root.activeFocus ? HydraTheme.borderFocus : HydraTheme.borderDark))
    clip: true
    scale: launchArea.pressed && root.canLaunch ? 0.98 : 1.0
    transformOrigin: Item.Center
    Accessible.role: Accessible.Button
    Accessible.name: "Launch tmux shell"
    Accessible.onPressAction: {
        if (root.canLaunch) {
            root.triggered()
        }
    }

    Keys.onPressed: event => {
        if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space)
                && root.canLaunch) {
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

    SequentialAnimation {
        id: armBurst

        NumberAnimation {
            target: armedBurst
            property: "opacity"
            from: 0.0
            to: 0.34
            duration: 110
            easing.type: Easing.OutCubic
        }

        NumberAnimation {
            target: armedBurst
            property: "opacity"
            from: 0.34
            to: 0.0
            duration: 260
            easing.type: Easing.InCubic
        }
    }

    MouseArea {
        id: launchArea

        anchors.fill: parent
        enabled: root.canLaunch
        hoverEnabled: true
        onContainsMouseChanged: if (containsMouse) HydraSounds.playHover()
        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onEntered: {
            root.hovered = true
            if (root.hoverHost && root.hoverHost.queueHoverHint) {
                root.hoverHost.queueHoverHint(root.canLaunch ? root.activeHoverText : root.inactiveHoverText,
                                              root)
            }
            if (root.armed) {
                root.playArmBurst()
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

    Rectangle {
        id: armedHalo

        anchors.fill: parent
        anchors.margins: 2
        radius: HydraTheme.radius6
        color: "transparent"
        border.width: root.armed || root.activeFocus ? 1 : 0
        border.color: HydraTheme.withAlpha(HydraTheme.accentCream, 0.84)
        opacity: root.armed
                 ? (root.activeFocus ? 0.48 : (root.hovered ? 0.24 : 0.16))
                 : (root.activeFocus ? 0.24 : 0.0)
    }

    Rectangle {
        id: armedField

        anchors.fill: parent
        color: HydraTheme.withAlpha(HydraTheme.accentOrangeStrong, 0.24)
        opacity: root.armed ? (root.hovered ? 0.12 : 0.08) : 0.0
    }

    Rectangle {
        id: armedBurst

        anchors.fill: parent
        color: HydraTheme.withAlpha(HydraTheme.accentCream, 0.16)
        opacity: 0.0
    }

    Rectangle {
        id: armedSweep

        x: -parent.width
        width: parent.width * 0.34
        height: parent.height
        color: HydraTheme.withAlpha(HydraTheme.accentCream, 0.2)
        visible: root.armed

        function restartSweep() {
            if (!root.armed) {
                return
            }
            x = -root.width
            armedSweepAnimation.restart()
        }

        NumberAnimation {
            id: armedSweepAnimation
            target: armedSweep
            property: "x"
            from: -root.width
            to: root.width * 1.2
            duration: 760
            easing.type: Easing.OutCubic
        }
    }

    onArmedChanged: {
        if (root.armed) {
            playArmBurst()
        }
    }

    FrameCorners {
        anchors.fill: parent
        lineColor: HydraTheme.withAlpha(HydraTheme.shellBg, 0.32)
        spanLength: 16
        spanHeight: 12
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: HydraTheme.space10
        spacing: HydraTheme.space10

        Rectangle {
            id: armedLead

            Layout.preferredWidth: 2
            Layout.fillHeight: true
            color: root.canLaunch ? HydraTheme.accentOrangeStrong : HydraTheme.borderDark
            opacity: root.armed ? 0.84 : 1.0
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0

            Text {
                text: root.titleText
                color: root.canLaunch ? HydraTheme.accentOrangeStrong : HydraTheme.textOnDarkMuted
                font.family: HydraTheme.displayFamily
                font.pixelSize: 11
                font.bold: true
                font.letterSpacing: 1.0
            }

            Text {
                visible: !root.denseMode
                text: root.subtitleText
                color: root.canLaunch
                       ? HydraTheme.withAlpha(HydraTheme.textOnDark, 0.7)
                       : HydraTheme.textOnDarkMuted
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        StatusChip {
            id: armedChip

            visible: root.armed
            opacity: root.hovered || root.activeFocus ? 0.92 : 0.82
            scale: 1.0
            toneColor: HydraTheme.accentOrangeStrong
            textColor: HydraTheme.accentCream
            fillOpacity: 0.18
            borderOpacity: 0.46
            minWidth: 64
            horizontalPadding: HydraTheme.space10
            verticalPadding: HydraTheme.space6
            textFamily: HydraTheme.displayFamily
            textPixelSize: 10
            textLetterSpacing: 0.9
            text: "ARMED"
        }
    }
}
