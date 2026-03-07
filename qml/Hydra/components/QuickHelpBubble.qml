pragma ComponentBehavior: Bound
import QtQuick 6.5
import "../styles"

Item {
    id: root

    property var topicData: undefined
    property string topicId: ""
    property real bubbleX: 0
    property real bubbleY: 0
    property int bubbleWidth: 320

    signal detailRequested(string topicId)
    signal dismissRequested()

    visible: !!root.topicData && !!root.topicData.title

    MouseArea {
        anchors.fill: parent
        onClicked: root.dismissRequested()
    }

    Rectangle {
        id: bubble

        x: root.bubbleX
        y: root.bubbleY
        width: root.bubbleWidth
        height: content.implicitHeight + (HydraTheme.space12 * 2)
        radius: HydraTheme.radius8
        color: HydraTheme.boardPanel
        border.width: 1
        border.color: HydraTheme.borderFocus
        opacity: root.visible ? 1.0 : 0.0
        scale: root.visible ? 1.0 : 0.97
        clip: true
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

        Column {
            id: content

            anchors.fill: parent
            anchors.margins: HydraTheme.space12
            spacing: HydraTheme.space8

            Text {
                text: root.topicData ? root.topicData.title : ""
                color: HydraTheme.textOnDark
                font.family: HydraTheme.displayFamily
                font.pixelSize: 16
                font.bold: true
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Text {
                text: root.topicData ? root.topicData.brief : ""
                color: HydraTheme.accentBronze
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Text {
                text: root.topicData ? root.topicData.summary : ""
                color: HydraTheme.textOnDarkMuted
                font.family: HydraTheme.bodyFamily
                font.pixelSize: 11
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Row {
                spacing: HydraTheme.space8

                Rectangle {
                    width: 94
                    height: 28
                    radius: HydraTheme.radius6
                    color: detailArea.pressed
                           ? HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.2)
                           : HydraTheme.withAlpha(HydraTheme.accentSteel,
                                                  detailArea.containsMouse ? 0.16 : 0.08)
                    border.width: 1
                    border.color: detailArea.containsMouse ? HydraTheme.borderFocus : HydraTheme.borderDark
                    Accessible.role: Accessible.Button
                    Accessible.name: "Open detailed help"
                    Accessible.onPressAction: root.detailRequested(root.topicId)

                    MouseArea {
                        id: detailArea

                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.detailRequested(root.topicId)
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "[DETAILS]"
                        color: detailArea.containsMouse ? HydraTheme.accentPhosphor : HydraTheme.textOnDark
                        font.family: HydraTheme.monoFamily
                        font.pixelSize: 9
                        font.bold: true
                    }
                }

                Rectangle {
                    width: 94
                    height: 28
                    radius: HydraTheme.radius6
                    color: dismissArea.pressed
                           ? HydraTheme.withAlpha(HydraTheme.accentBronze, 0.18)
                           : HydraTheme.withAlpha(HydraTheme.accentBronze,
                                                  dismissArea.containsMouse ? 0.12 : 0.06)
                    border.width: 1
                    border.color: dismissArea.containsMouse ? HydraTheme.borderFocus : HydraTheme.borderDark
                    Accessible.role: Accessible.Button
                    Accessible.name: "Dismiss quick help"
                    Accessible.onPressAction: root.dismissRequested()

                    MouseArea {
                        id: dismissArea

                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.dismissRequested()
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "[DISMISS]"
                        color: dismissArea.containsMouse ? HydraTheme.accentBronze : HydraTheme.textOnDarkMuted
                        font.family: HydraTheme.monoFamily
                        font.pixelSize: 9
                        font.bold: true
                    }
                }
            }
        }
    }
}
