pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Controls.Basic 6.5 as BasicControls
import QtQuick.Layouts 6.5
import "../styles"

Item {
    id: root

    property string selectedKey: ""
    property string selectedLabel: ""
    property bool selectionAvailable: true
    property var optionModel: null
    property var helpHost: null
    property int maxVisibleRows: 6
    readonly property int selectorHeight: 34
    readonly property int popupGap: HydraTheme.space4

    signal optionSelected(string optionKey)

    implicitWidth: providerBox.implicitWidth
    implicitHeight: root.selectorHeight

    BasicControls.ComboBox {
        id: providerBox

        anchors.fill: parent
        model: root.optionModel
        textRole: "displayName"
        valueRole: "providerKey"
        currentIndex: indexOfValue(root.selectedKey)
        displayText: root.selectedLabel.length > 0 ? root.selectedLabel : currentText
        leftPadding: HydraTheme.space12
        rightPadding: 84
        topPadding: 0
        bottomPadding: 0
        hoverEnabled: true
        onHoveredChanged: if (hovered) HydraSounds.playHover()
        implicitHeight: root.selectorHeight
        font.family: HydraTheme.displayFamily
        font.pixelSize: 10
        font.bold: true
        font.letterSpacing: 0.7
        Accessible.role: Accessible.ComboBox
        Accessible.name: "Open provider selector"

        onActivated: root.optionSelected(currentValue)

        Connections {
            target: providerPopup

            function onVisibleChanged() {
                if (providerPopup.visible && root.helpHost && root.helpHost.clearHoverHint) {
                    root.helpHost.clearHoverHint(providerBox)
                }
            }
        }

        HoverHandler {
            onHoveredChanged: {
                if (!root.helpHost) {
                    return
                }
                if (providerBox.popup.visible) {
                    if (root.helpHost.clearHoverHint) {
                        root.helpHost.clearHoverHint(providerBox)
                    }
                    return
                }
                if (hovered && root.helpHost.queueHoverHint) {
                    root.helpHost.queueHoverHint(
                        "Choose which installed CLI Hydra should launch for the current target.",
                        providerBox)
                } else if (!hovered && root.helpHost.clearHoverHint) {
                    root.helpHost.clearHoverHint(providerBox)
                }
            }
        }

        indicator: Item {
            implicitWidth: 0
            implicitHeight: 0
        }

        contentItem: Text {
            text: providerBox.displayText
            color: root.selectionAvailable ? HydraTheme.textOnDark : HydraTheme.danger
            font.family: HydraTheme.displayFamily
            font.pixelSize: 10
            font.bold: true
            font.letterSpacing: 0.7
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            radius: HydraTheme.radius6
            color: providerBox.hovered
                   ? HydraTheme.withAlpha(HydraTheme.accentBronze, 0.12)
                   : HydraTheme.withAlpha(HydraTheme.textOnDark, 0.05)
            border.width: 1
            border.color: providerBox.popup.visible
                          ? HydraTheme.withAlpha(HydraTheme.accentBronze, 0.76)
                          : (root.selectionAvailable
                             ? HydraTheme.borderDark
                             : HydraTheme.withAlpha(HydraTheme.danger, 0.52))

            Behavior on color {
                ColorAnimation { duration: HydraTheme.motionFast }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: HydraTheme.space8
                width: 2
                radius: 1
                color: root.selectionAvailable ? HydraTheme.accentBronze : HydraTheme.danger
            }

            Text {
                id: indicatorGlyph

                anchors.right: parent.right
                anchors.rightMargin: HydraTheme.space12
                anchors.verticalCenter: parent.verticalCenter
                text: providerBox.popup.visible ? "˄" : "˅"
                color: HydraTheme.accentBronze
                font.family: HydraTheme.displayFamily
                font.pixelSize: 15
                font.bold: true
            }

            StatusChip {
                anchors.right: indicatorGlyph.left
                anchors.rightMargin: HydraTheme.space10
                anchors.verticalCenter: parent.verticalCenter
                toneColor: root.selectionAvailable ? HydraTheme.accentReady : HydraTheme.danger
                textColor: toneColor
                fillOpacity: 0.08
                borderOpacity: 0.28
                minWidth: 50
                horizontalPadding: HydraTheme.space4
                verticalPadding: HydraTheme.space2
                textPixelSize: 10
                text: root.selectionAvailable ? "READY" : "MISSING"
            }
        }

        delegate: ItemDelegate {
            id: optionDelegate

            required property var model
            required property int index

            width: ListView.view ? ListView.view.width : providerBox.width
            height: 30
            padding: 0
            opacity: model.available || model.selected ? 1.0 : 0.62
            highlighted: providerBox.highlightedIndex === index
            Accessible.role: Accessible.Button
            Accessible.name: "Select provider " + model.displayName

            background: Rectangle {
                radius: HydraTheme.radius4
                color: optionDelegate.highlighted
                       ? HydraTheme.withAlpha(HydraTheme.textOnDark, 0.1)
                       : (optionDelegate.model.selected
                          ? HydraTheme.withAlpha(HydraTheme.accentBronze, 0.18)
                          : HydraTheme.withAlpha(HydraTheme.textOnDark, 0.03))
                border.width: 1
                border.color: optionDelegate.model.selected
                              ? HydraTheme.withAlpha(HydraTheme.accentBronze, 0.72)
                              : (optionDelegate.model.available
                                 ? HydraTheme.borderDark
                                 : HydraTheme.withAlpha(HydraTheme.danger, 0.46))
            }

            contentItem: RowLayout {
                anchors.fill: parent
                anchors.leftMargin: HydraTheme.space6
                anchors.rightMargin: HydraTheme.space10
                anchors.topMargin: HydraTheme.space6
                anchors.bottomMargin: HydraTheme.space6
                spacing: HydraTheme.space6

                Text {
                    Layout.fillWidth: true
                    text: optionDelegate.model.displayName
                    color: optionDelegate.model.available ? HydraTheme.textOnDark : HydraTheme.danger
                    font.family: HydraTheme.displayFamily
                    font.pixelSize: 10
                    font.bold: true
                    font.letterSpacing: 0.65
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }

                StatusChip {
                    Layout.rightMargin: HydraTheme.space8
                    toneColor: optionDelegate.model.available ? HydraTheme.accentReady : HydraTheme.danger
                    textColor: toneColor
                    fillOpacity: 0.08
                    borderOpacity: 0.28
                    minWidth: 50
                    horizontalPadding: HydraTheme.space4
                    verticalPadding: HydraTheme.space2
                    textPixelSize: 10
                    text: optionDelegate.model.available ? "READY" : "MISSING"
                }
            }
        }

        popup: Popup {
            id: providerPopup
            parent: Overlay.overlay
            readonly property real desiredHeight: providerList.implicitHeight + (padding * 2)

            x: {
                if (!providerPopup.parent) {
                    return 0
                }
                const popupOrigin = providerBox.mapToItem(providerPopup.parent, 0, 0)
                const maxX = Math.max(HydraTheme.space8,
                                      providerPopup.parent.width - providerPopup.width - HydraTheme.space8)
                return Math.max(HydraTheme.space8, Math.min(maxX, popupOrigin.x))
            }
            y: {
                if (!providerPopup.parent) {
                    return providerBox.height + root.popupGap
                }
                const popupOrigin = providerBox.mapToItem(providerPopup.parent, 0, 0)
                const belowY = popupOrigin.y + providerBox.height + root.popupGap
                const aboveY = popupOrigin.y - providerPopup.desiredHeight - root.popupGap
                const maxY = Math.max(HydraTheme.space8,
                                      providerPopup.parent.height - providerPopup.desiredHeight - HydraTheme.space8)
                if (belowY + providerPopup.desiredHeight <= providerPopup.parent.height - HydraTheme.space8
                        || aboveY < HydraTheme.space8) {
                    return Math.max(HydraTheme.space8, Math.min(maxY, belowY))
                }
                return Math.max(HydraTheme.space8, aboveY)
            }
            width: providerBox.width
            padding: HydraTheme.space4
            modal: false
            dim: false
            focus: true
            z: 200
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

            contentItem: ListView {
                id: providerList

                implicitHeight: Math.min(contentHeight,
                                         (30 * root.maxVisibleRows)
                                         + (HydraTheme.space3 * Math.max(0, root.maxVisibleRows - 1)))
                model: providerBox.popup.visible ? providerBox.delegateModel : null
                currentIndex: providerBox.highlightedIndex
                spacing: HydraTheme.space3
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                interactive: contentHeight > height
                ScrollBar.vertical: ScrollBar {
                    policy: providerList.contentHeight > providerList.height
                            ? ScrollBar.AlwaysOn
                            : ScrollBar.AlwaysOff
                }
            }

            background: Rectangle {
                radius: HydraTheme.radius6
                color: HydraTheme.railPanelStrong
                border.width: 1
                border.color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.5)

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 1
                    radius: HydraTheme.radius6
                    color: HydraTheme.shellDepth
                    border.width: 1
                    border.color: HydraTheme.withAlpha(HydraTheme.borderDark, 0.78)
                }
            }

            enter: Transition {
                ParallelAnimation {
                    NumberAnimation {
                        property: "opacity"
                        from: 0.0
                        to: 1.0
                        duration: HydraTheme.motionFast
                    }

                    NumberAnimation {
                        property: "scale"
                        from: 0.98
                        to: 1.0
                        duration: HydraTheme.motionNormal
                        easing.type: Easing.OutCubic
                    }
                }
            }

            exit: Transition {
                ParallelAnimation {
                    NumberAnimation {
                        property: "opacity"
                        from: 1.0
                        to: 0.0
                        duration: HydraTheme.motionFast
                    }

                    NumberAnimation {
                        property: "scale"
                        from: 1.0
                        to: 0.985
                        duration: HydraTheme.motionFast
                        easing.type: Easing.InCubic
                    }
                }
            }
        }
    }
}
