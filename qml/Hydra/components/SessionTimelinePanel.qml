pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import "../styles"

Rectangle {
    id: root

    property var entries: []
    property bool denseMode: false
    property var hoverHost: null

    function toneColorForKey(toneKey) {
        switch (toneKey) {
        case "ready":
            return HydraTheme.accentReady
        case "bronze":
            return HydraTheme.accentBronze
        case "phosphor":
            return HydraTheme.accentPhosphor
        case "danger":
            return HydraTheme.danger
        case "steel":
        default:
            return HydraTheme.accentSteelBright
        }
    }

    implicitHeight: contentColumn.implicitHeight + (root.denseMode ? HydraTheme.space10 : HydraTheme.space12)
    radius: HydraTheme.radius6
    color: HydraTheme.withAlpha(HydraTheme.boardPanelMuted, 0.9)
    border.width: 1
    border.color: HydraTheme.withAlpha(HydraTheme.borderDark, 0.88)
    clip: true

    ColumnLayout {
        id: contentColumn

        anchors.fill: parent
        anchors.margins: root.denseMode ? HydraTheme.space8 : HydraTheme.space10
        spacing: root.denseMode ? HydraTheme.space6 : HydraTheme.space8

        RowLayout {
            Layout.fillWidth: true
            spacing: HydraTheme.space8

            Text {
                text: "TRACE"
                color: HydraTheme.textOnDark
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 0.8
            }

            StatusChip {
                toneColor: HydraTheme.accentSteelBright
                textColor: HydraTheme.textOnDark
                fillOpacity: 0.08
                borderOpacity: 0.24
                horizontalPadding: root.denseMode ? HydraTheme.space6 : HydraTheme.space8
                verticalPadding: HydraTheme.space2
                textPixelSize: 10
                text: root.entries.length + " EVENTS"
                toolTipText: "Recent persisted observability events for this detached session."
                hoverHost: root.hoverHost
            }

            Item {
                Layout.fillWidth: true
            }
        }

        Repeater {
            model: root.entries

            delegate: Rectangle {
                id: entryDelegate
                required property var modelData

                Layout.fillWidth: true
                radius: HydraTheme.radius4
                color: HydraTheme.withAlpha(HydraTheme.boardRow, 0.72)
                border.width: 1
                border.color: HydraTheme.withAlpha(HydraTheme.borderDark, 0.62)
                implicitHeight: entryColumn.implicitHeight + (root.denseMode ? HydraTheme.space8 : HydraTheme.space10)

                ColumnLayout {
                    id: entryColumn

                    anchors.fill: parent
                    anchors.margins: root.denseMode ? HydraTheme.space6 : HydraTheme.space8
                    spacing: HydraTheme.space4

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: HydraTheme.space8

                        Text {
                            text: entryDelegate.modelData.summary
                            color: HydraTheme.textOnDark
                            font.family: HydraTheme.bodyFamily
                            font.pixelSize: root.denseMode ? 10 : 11
                            font.bold: true
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        StatusChip {
                            toneColor: root.toneColorForKey(entryDelegate.modelData.provenanceTone)
                            textColor: root.toneColorForKey(entryDelegate.modelData.provenanceTone)
                            fillOpacity: 0.08
                            borderOpacity: 0.26
                            horizontalPadding: root.denseMode ? HydraTheme.space4 : HydraTheme.space6
                            verticalPadding: HydraTheme.space2
                            textPixelSize: 10
                            text: entryDelegate.modelData.provenanceLabel.toUpperCase()
                            toolTipText: "Signal source used for this status update."
                            hoverHost: root.hoverHost
                        }

                        Text {
                            text: entryDelegate.modelData.occurredAtText
                            color: HydraTheme.textOnLightSoft
                            font.family: HydraTheme.monoFamily
                            font.pixelSize: 10
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: entryDelegate.modelData.detail
                        color: HydraTheme.textOnDarkMuted
                        font.family: HydraTheme.monoFamily
                        font.pixelSize: 10
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        Layout.fillWidth: true
                        text: "STATE // " + entryDelegate.modelData.stateLabel.toUpperCase()
                        color: HydraTheme.textOnLightSoft
                        font.family: HydraTheme.monoFamily
                        font.pixelSize: 10
                        font.letterSpacing: 0.5
                    }
                }
            }
        }
    }
}
