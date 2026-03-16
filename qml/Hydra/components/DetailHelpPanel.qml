pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import "../styles"

Item {
    id: root

    property var topicData: undefined
    property int panelWidth: 720
    property int panelHeight: 640

    signal closeRequested()

    visible: !!root.topicData && !!root.topicData.title

    Rectangle {
        anchors.fill: parent
        color: HydraTheme.withAlpha(HydraTheme.shellDepth, 0.72)
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.closeRequested()
    }

    Rectangle {
        id: panel

        width: root.panelWidth
        height: root.panelHeight
        anchors.centerIn: parent
        radius: HydraTheme.radius10
        color: HydraTheme.railPanelStrong
        border.width: 1
        border.color: HydraTheme.withAlpha(HydraTheme.accentReady, 0.72)
        clip: true
        scale: root.visible ? 1.0 : 0.98
        opacity: root.visible ? 1.0 : 0.0
        z: 1

        Behavior on opacity {
            NumberAnimation { duration: HydraTheme.motionNormal }
        }

        Behavior on scale {
            NumberAnimation {
                duration: HydraTheme.motionNormal
                easing.type: Easing.OutCubic
            }
        }

        MouseArea {
            anchors.fill: parent
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 2
            color: HydraTheme.withAlpha(HydraTheme.accentReady, 0.62)
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: HydraTheme.radius8
            color: "transparent"
            border.width: 1
            border.color: HydraTheme.withAlpha(HydraTheme.borderDark, 0.72)
        }

        Column {
            anchors.fill: parent
            anchors.margins: HydraTheme.space14
            spacing: HydraTheme.space10

            Row {
                width: parent.width
                spacing: HydraTheme.space8

                Column {
                    width: parent.width - closeButton.width - HydraTheme.space8
                    spacing: HydraTheme.space4

                    Text {
                        text: root.topicData ? root.topicData.title : ""
                        color: HydraTheme.textOnDark
                        font.family: HydraTheme.displayFamily
                        font.pixelSize: 22
                        font.bold: true
                        width: parent.width
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        text: "current shell documentation // live build"
                        color: HydraTheme.accentSteelBright
                        font.family: HydraTheme.monoFamily
                        font.pixelSize: 10
                        width: parent.width
                        wrapMode: Text.WordWrap
                    }
                }

                Rectangle {
                    id: closeButton

                    width: 98
                    height: 34
                    radius: HydraTheme.radius6
                    color: closeArea.pressed
                           ? HydraTheme.withAlpha(HydraTheme.accentBronze, 0.24)
                           : HydraTheme.withAlpha(HydraTheme.accentBronze,
                                                  closeArea.containsMouse ? 0.16 : 0.09)
                    border.width: 1
                    border.color: closeArea.containsMouse ? HydraTheme.accentBronze : HydraTheme.borderFocus
                    Accessible.role: Accessible.Button
                    Accessible.name: "Close detailed help"
                    Accessible.onPressAction: root.closeRequested()

                    MouseArea {
                        id: closeArea

                        anchors.fill: parent
                        hoverEnabled: true
                        onContainsMouseChanged: if (containsMouse) HydraSounds.playHover()
                        cursorShape: Qt.PointingHandCursor
                        onClicked: { HydraSounds.playClick(); root.closeRequested() }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "[CLOSE]"
                        color: closeArea.containsMouse ? HydraTheme.accentPhosphor : HydraTheme.accentBronze
                        font.family: HydraTheme.monoFamily
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                color: HydraTheme.withAlpha(HydraTheme.borderDark, 0.88)
            }

            ScrollView {
                width: parent.width
                height: parent.height - y
                clip: true

                Column {
                    width: panel.width - (HydraTheme.space14 * 2) - HydraTheme.space8
                    spacing: HydraTheme.space12

                    Text {
                        text: root.topicData ? root.topicData.summary : ""
                        color: HydraTheme.textOnDark
                        font.family: HydraTheme.bodyFamily
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }

                    HelpSectionCard {
                        title: "USE IT NOW"
                        accentColor: HydraTheme.accentBronze
                        lines: root.topicData ? root.topicData.useNow : []
                    }

                    HelpSectionCard {
                        title: "CONTROLS IN THIS SECTION"
                        accentColor: HydraTheme.accentSteelBright
                        lines: root.topicData ? root.topicData.controls : []
                    }

                    HelpSectionCard {
                        visible: Boolean(root.topicData && root.topicData.hotkeys
                                         && root.topicData.hotkeys.length > 0)
                        title: "KEYBOARD SHORTCUTS"
                        accentColor: HydraTheme.accentPhosphor
                        lines: {
                            if (!root.topicData || !root.topicData.hotkeys)
                                return []
                            return root.topicData.hotkeys.map(function(h) {
                                return h.key + "  \u2014  " + h.action
                            })
                        }
                    }

                    HelpSectionCard {
                        visible: Boolean(root.topicData && root.topicData.limits
                                         && root.topicData.limits.length > 0)
                        title: "KNOWN LIMITS"
                        accentColor: HydraTheme.warning
                        lines: root.topicData ? root.topicData.limits : []
                    }

                }
            }
        }
    }
}
