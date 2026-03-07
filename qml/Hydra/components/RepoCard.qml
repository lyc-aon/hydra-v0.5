import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts 6.5
import "../styles"

Rectangle {
    id: root

    property string repoId: ""
    property string repoName: ""
    property string repoPath: ""
    property string repoDescription: ""
    property color accentColor: HydraTheme.accentBronze
    property bool selected: false
    property QtObject hoverHost: null
    property bool compactMode: false
    property bool hovered: false

    signal activated()

    radius: HydraTheme.radius6
    implicitHeight: root.compactMode ? 54 : (repoDescription.length > 0 ? 78 : 62)
    color: selected
           ? HydraTheme.withAlpha(root.accentColor, root.hovered ? 0.16 : 0.11)
           : (root.hovered
              ? HydraTheme.withAlpha(HydraTheme.railCardSelected, 0.98)
              : HydraTheme.railCard)
    border.width: 1
    border.color: selected
                  ? HydraTheme.withAlpha(accentColor, root.hovered ? 0.72 : 0.58)
                  : (root.hovered ? HydraTheme.borderFocus : HydraTheme.borderDark)
    clip: true
    scale: root.hovered ? 1.008 : 1.0
    Accessible.role: Accessible.Button
    Accessible.name: repoName.length > 0 ? "Repository " + repoName : "Repository"
    Accessible.onPressAction: root.activated()

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
        color: selected ? root.accentColor : HydraTheme.withAlpha(root.accentColor, 0.55)
    }

    Rectangle {
        anchors.left: edgeBar.right
        anchors.right: parent.right
        anchors.top: parent.top
        height: 1
        color: HydraTheme.withAlpha(selected ? root.accentColor : HydraTheme.accentSteel, 0.24)
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
                root.hoverHost.queueHoverHint("Select this repository as the current launch context.", root)
            }
        }
        onExited: {
            root.hovered = false
            if (root.hoverHost && root.hoverHost.clearHoverHint) {
                root.hoverHost.clearHoverHint(root)
            }
        }
        onClicked: root.activated()
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
                text: root.repoName
                color: HydraTheme.textOnDark
                font.family: HydraTheme.displayFamily
                font.pixelSize: root.compactMode ? 12 : 14
                font.bold: true
                font.letterSpacing: 0.8
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Text {
                visible: root.selected
                text: root.compactMode ? "[TGT]" : "[TARGET]"
                color: root.accentColor
                font.family: HydraTheme.monoFamily
                font.pixelSize: 8
                font.bold: true
            }
        }

        Text {
            visible: !root.compactMode && root.repoDescription.length > 0
            text: root.repoDescription
            color: HydraTheme.textOnDarkMuted
            font.family: HydraTheme.monoFamily
            font.pixelSize: 9
            wrapMode: Text.WordWrap
            maximumLineCount: 1
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        Text {
            text: root.repoPath
            color: HydraTheme.accentSteelBright
            font.pixelSize: root.compactMode ? 8 : 8
            font.family: HydraTheme.monoFamily
            elide: Text.ElideMiddle
            Layout.fillWidth: true
        }
    }
}
