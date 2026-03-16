import QtQuick 6.5
import QtQuick.Layouts 6.5
import "../styles"

Rectangle {
    id: root

    property string branchName: ""
    property string path: ""
    property bool isMain: false
    property bool selected: false
    property var hoverHost: null
    property bool compactMode: false
    property bool hovered: false
    readonly property color removeToneColor: HydraTheme.currentThemeId === "og_steam"
                                             ? HydraTheme.accentSignal
                                             : HydraTheme.danger
    readonly property color linkedToneColor: HydraTheme.accentSteelBright
    readonly property color targetToneColor: HydraTheme.accentBronze
    readonly property int inlineTagHeight: linkedTag.implicitHeight

    signal activated(string path)
    signal removeRequested(string path)

    radius: HydraTheme.radius6
    implicitHeight: root.compactMode ? 44 : 48
    color: selected
           ? HydraTheme.withAlpha(root.isMain ? HydraTheme.accentReady : HydraTheme.accentBronze,
                                  root.hovered ? 0.16 : 0.11)
           : (root.hovered
              ? HydraTheme.withAlpha(HydraTheme.railCardSelected, 0.98)
              : HydraTheme.railCard)
    border.width: 1
    border.color: selected
                  ? HydraTheme.withAlpha(HydraTheme.accentBronze,
                                         root.activeFocus ? 0.86 : (root.hovered ? 0.72 : 0.58))
                  : ((root.hovered || root.activeFocus) ? HydraTheme.borderFocus : HydraTheme.borderDark)
    clip: true
    activeFocusOnTab: true
    scale: root.hovered ? 1.008 : 1.0
    Accessible.role: Accessible.Button
    Accessible.name: branchName.length > 0 ? "Worktree " + branchName : "Worktree"
    Accessible.onPressAction: root.activated(root.path)

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
        id: edgeBar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: root.selected ? 3 : 2
        color: root.isMain ? HydraTheme.accentReadyDeep : HydraTheme.accentBronze
    }

    Rectangle {
        anchors.left: edgeBar.right
        anchors.right: parent.right
        anchors.top: parent.top
        height: 1
        color: HydraTheme.withAlpha(root.isMain ? HydraTheme.accentReady : HydraTheme.accentBronze, 0.24)
    }

    Rectangle {
        anchors.fill: parent
        color: HydraTheme.withAlpha(HydraTheme.accentSteel, root.hovered ? 0.04 : 0.0)

        Behavior on color {
            ColorAnimation { duration: HydraTheme.motionFast }
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onContainsMouseChanged: if (containsMouse) HydraSounds.playHover()
        cursorShape: Qt.PointingHandCursor
        onEntered: {
            root.hovered = true
            if (root.hoverHost && root.hoverHost.queueHoverHint) {
                root.hoverHost.queueHoverHint("Route new shells into this linked worktree path.", root)
            }
        }
        onExited: {
            root.hovered = false
            if (root.hoverHost && root.hoverHost.clearHoverHint) {
                root.hoverHost.clearHoverHint(root)
            }
        }
        onClicked: { HydraSounds.playClick(); root.activated(root.path) }
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
            root.activated(root.path)
            event.accepted = true
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: root.compactMode ? HydraTheme.space10 : HydraTheme.space10
        anchors.rightMargin: root.compactMode ? HydraTheme.space10 : HydraTheme.space10
        anchors.topMargin: root.compactMode ? HydraTheme.space6 : HydraTheme.space8
        anchors.bottomMargin: root.compactMode ? HydraTheme.space6 : HydraTheme.space8
        spacing: HydraTheme.space4

        RowLayout {
            Layout.fillWidth: true
            spacing: HydraTheme.space6

            Text {
                text: root.branchName.length > 0 ? root.branchName : "linked"
                color: root.isMain ? HydraTheme.accentReady : HydraTheme.textOnDark
                font.family: HydraTheme.displayFamily
                font.pixelSize: 11
                font.bold: true
                font.letterSpacing: 0.6
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            StatusChip {
                id: linkedTag

                text: root.isMain ? "MAIN" : "LINKED"
                toneColor: root.isMain ? HydraTheme.accentReady : root.linkedToneColor
                textColor: HydraTheme.textOnDark
                fillOpacity: root.isMain ? 0.18 : 0.12
                borderOpacity: root.isMain ? 0.42 : 0.3
                horizontalPadding: root.compactMode ? HydraTheme.space4 : HydraTheme.space5
                verticalPadding: HydraTheme.space2
                textPixelSize: 10
                minWidth: root.compactMode ? 42 : 54
            }

            StatusChip {
                visible: root.selected
                text: root.compactMode ? "TGT" : "TARGET"
                toneColor: root.targetToneColor
                textColor: HydraTheme.textOnDark
                fillOpacity: 0.2
                borderOpacity: 0.42
                horizontalPadding: root.compactMode ? HydraTheme.space4 : HydraTheme.space5
                verticalPadding: HydraTheme.space2
                textPixelSize: 10
                minWidth: root.compactMode ? 40 : 52
            }

            ActionChip {
                id: removeButton
                visible: !root.isMain
                text: root.compactMode ? "[X]" : "[REMOVE]"
                toneColor: root.removeToneColor
                minWidth: root.compactMode ? 46 : 0
                implicitHeight: root.inlineTagHeight
                accessibleLabel: root.branchName.length > 0
                                 ? "Remove worktree " + root.branchName
                                 : "Remove worktree"
                toolTipText: "Remove this linked worktree from Git and disk."
                hoverHost: root.hoverHost
                onClicked: root.removeRequested(root.path)
            }
        }

        Text {
            text: root.path
            color: HydraTheme.textOnLightSoft
            font.pixelSize: 10
            font.family: HydraTheme.monoFamily
            elide: Text.ElideMiddle
            Layout.fillWidth: true
        }
    }
}
