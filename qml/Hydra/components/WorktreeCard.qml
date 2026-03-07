import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts 6.5
import "../styles"

Rectangle {
    id: root

    property string branchName: ""
    property string path: ""
    property bool isMain: false
    property bool selected: false
    property QtObject hoverHost: null
    property bool compactMode: false
    property bool hovered: false

    signal activated(string path)

    radius: HydraTheme.radius6
    implicitHeight: root.compactMode ? 46 : 58
    color: selected
           ? HydraTheme.withAlpha(root.isMain ? HydraTheme.accentReady : HydraTheme.accentBronze,
                                  root.hovered ? 0.16 : 0.11)
           : (root.hovered
              ? HydraTheme.withAlpha(HydraTheme.railCardSelected, 0.98)
              : HydraTheme.railCard)
    border.width: 1
    border.color: selected
                  ? HydraTheme.withAlpha(HydraTheme.accentBronze, root.hovered ? 0.72 : 0.58)
                  : (root.hovered ? HydraTheme.borderFocus : HydraTheme.borderDark)
    clip: true
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
        width: selected ? 3 : 2
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
        onClicked: root.activated(root.path)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: root.compactMode ? HydraTheme.space8 : HydraTheme.space10
        anchors.rightMargin: root.compactMode ? HydraTheme.space8 : HydraTheme.space10
        anchors.topMargin: root.compactMode ? HydraTheme.space8 : HydraTheme.space9
        anchors.bottomMargin: root.compactMode ? HydraTheme.space8 : HydraTheme.space9
        spacing: root.compactMode ? HydraTheme.space2 : HydraTheme.space4

        RowLayout {
            Layout.fillWidth: true
            spacing: HydraTheme.space8

            Text {
                text: root.branchName.length > 0 ? root.branchName : "linked"
                color: root.isMain ? HydraTheme.accentReady : HydraTheme.textOnDark
                font.family: HydraTheme.displayFamily
                font.pixelSize: root.compactMode ? 11 : 12
                font.bold: true
                font.letterSpacing: 0.8
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Text {
                text: root.isMain ? "[MAIN]" : "[LINKED]"
                color: root.isMain ? HydraTheme.accentReady : HydraTheme.textOnDarkMuted
                font.family: HydraTheme.monoFamily
                font.pixelSize: root.compactMode ? 8 : 8
                font.bold: true
            }

            Text {
                visible: root.selected
                text: root.compactMode ? "[TGT]" : "[TARGET]"
                color: HydraTheme.accentBronze
                font.family: HydraTheme.monoFamily
                font.pixelSize: 8
                font.bold: true
            }
        }

        Text {
            visible: !root.compactMode
            text: root.path
            color: HydraTheme.accentSteelBright
            font.pixelSize: 8
            font.family: HydraTheme.monoFamily
            elide: Text.ElideMiddle
            Layout.fillWidth: true
        }
    }
}
