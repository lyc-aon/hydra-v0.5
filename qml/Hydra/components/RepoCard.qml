import QtQuick 6.5
import QtQuick.Layouts 6.5
import "../styles"

Rectangle {
    id: root

    property string repoId: ""
    property string repoName: ""
    property string repoPath: ""
    property color accentColor: HydraTheme.accentBronze
    property bool selected: false
    property var hoverHost: null
    property bool compactMode: false
    property bool hovered: false
    readonly property color effectiveAccentColor: HydraTheme.currentThemeId === "og_steam"
                                                 ? HydraTheme.repositoryAccent
                                                 : root.accentColor
    readonly property color targetToneColor: HydraTheme.accentBronze
    readonly property color removeToneColor: HydraTheme.currentThemeId === "og_steam"
                                             ? HydraTheme.accentSignal
                                             : HydraTheme.danger
    readonly property int inlineTagHeight: targetTag.implicitHeight

    signal activated()
    signal removeRequested(string repoId)

    radius: HydraTheme.radius6
    implicitHeight: root.compactMode ? 44 : 48
    color: selected
           ? HydraTheme.withAlpha(root.effectiveAccentColor, root.hovered ? 0.16 : 0.11)
           : (root.hovered
              ? HydraTheme.withAlpha(HydraTheme.railCardSelected, 0.98)
              : HydraTheme.railCard)
    border.width: 1
    border.color: selected
                  ? HydraTheme.withAlpha(root.effectiveAccentColor,
                                         root.activeFocus ? 0.86 : (root.hovered ? 0.72 : 0.58))
                  : ((root.hovered || root.activeFocus) ? HydraTheme.borderFocus : HydraTheme.borderDark)
    clip: true
    activeFocusOnTab: true
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
        width: root.selected ? 3 : 2
        color: root.selected ? root.effectiveAccentColor : HydraTheme.withAlpha(root.effectiveAccentColor, 0.55)
    }

    Rectangle {
        anchors.left: edgeBar.right
        anchors.right: parent.right
        anchors.top: parent.top
        height: 1
        color: HydraTheme.withAlpha(root.selected ? root.effectiveAccentColor : HydraTheme.accentSteel, 0.24)
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
                root.hoverHost.queueHoverHint("Select this repository as the current launch context.", root)
            }
        }
        onExited: {
            root.hovered = false
            if (root.hoverHost && root.hoverHost.clearHoverHint) {
                root.hoverHost.clearHoverHint(root)
            }
        }
        onClicked: { HydraSounds.playClick(); root.activated() }
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
            root.activated()
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
                text: root.repoName
                color: HydraTheme.textOnDark
                font.family: HydraTheme.displayFamily
                font.pixelSize: root.compactMode ? 11 : 12
                font.bold: true
                font.letterSpacing: 0.6
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            StatusChip {
                id: targetTag

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
                text: root.compactMode ? "[X]" : "[UNLINK]"
                toneColor: root.removeToneColor
                minWidth: root.compactMode ? 46 : 0
                implicitHeight: root.inlineTagHeight
                accessibleLabel: root.repoName.length > 0
                                 ? "Remove repository " + root.repoName
                                 : "Remove repository"
                toolTipText: "Remove this repository from Hydra. Files on disk stay intact."
                hoverHost: root.hoverHost
                onClicked: root.removeRequested(root.repoId)
            }
        }

        Text {
            text: root.repoPath
            color: HydraTheme.textOnLightSoft
            font.pixelSize: 10
            font.family: HydraTheme.monoFamily
            elide: Text.ElideMiddle
            Layout.fillWidth: true
        }
    }
}
