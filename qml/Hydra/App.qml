pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Window 6.5
import "HelpCatalog.js" as HelpCatalog
import "components"
import "styles"

Window {
    id: root

    required property QtObject appState
    property bool startupSidebarCollapsed: false
    property int startupSidebarWidth: -1
    property string startupQuickHelpTopic: ""
    property string startupDetailHelpTopic: ""
    property bool sidebarCollapsed: startupSidebarCollapsed
    property real sidebarReveal: startupSidebarCollapsed ? 0.0 : 1.0
    property real dividerBaseWidth: 0
    property real expandedRailWidth: Math.max(minRailWidth,
                                              Math.min(maxRailWidth,
                                                       startupSidebarWidth > 0 ? startupSidebarWidth : railWidth))
    property real storedRailWidth: expandedRailWidth
    property string quickHelpTopicId: ""
    property string detailHelpTopicId: ""
    property string hoverHintText: ""
    property Item hoverHintSource: null
    property real quickHelpX: 0
    property real quickHelpY: 0
    property real hoverHintX: 0
    property real hoverHintY: 0
    readonly property bool compactShell: width < 1420
    readonly property bool narrowShell: width < 1220
    readonly property bool tightShell: width < 1080
    readonly property int railWidth: tightShell ? 248 : (narrowShell ? 276 : (compactShell ? 308 : 356))
    readonly property int minRailWidth: tightShell ? 208 : (narrowShell ? 228 : (compactShell ? 256 : 288))
    readonly property int maxRailWidth: tightShell ? 300 : (narrowShell ? 344 : (compactShell ? 392 : 452))
    readonly property int dividerWidth: tightShell ? 22 : 26
    readonly property int shellInset: tightShell ? HydraTheme.space10 : HydraTheme.shellMargin
    readonly property int panelInset: tightShell ? HydraTheme.space10 : HydraTheme.space12
    readonly property int frameRadius: tightShell ? HydraTheme.radius10 : HydraTheme.radius12
    readonly property int quickHelpWidth: tightShell ? 272 : 320
    readonly property int detailHelpWidth: Math.min(720, width - (shellInset * 2) - HydraTheme.space32)
    readonly property int detailHelpHeight: Math.min(640, height - (shellInset * 2) - HydraTheme.space32)
    readonly property int hoverHintWidth: tightShell ? 212 : 248
    readonly property real railVisibleWidth: Math.round(expandedRailWidth * sidebarReveal)
    readonly property real boardLayoutWidth: shellSurface.width - dividerWidth - (sidebarCollapsed ? 0 : expandedRailWidth)
    readonly property var quickHelpData: HelpCatalog.topic(quickHelpTopicId)
    readonly property var detailHelpData: HelpCatalog.topic(detailHelpTopicId)

    width: 1460
    height: 920
    minimumWidth: 960
    minimumHeight: 680
    visible: true
    title: "Hydra V2"
    color: HydraTheme.shellBg

    function clampRailWidth(value) {
        return Math.max(minRailWidth, Math.min(maxRailWidth, value))
    }

    function syncRailWidthBounds() {
        const baseWidth = clampRailWidth(railWidth)
        storedRailWidth = clampRailWidth(storedRailWidth > 0 ? storedRailWidth : baseWidth)
        if (!sidebarCollapsed) {
            expandedRailWidth = clampRailWidth(expandedRailWidth > 0 ? expandedRailWidth : baseWidth)
        }
    }

    function toggleSidebar() {
        if (sidebarCollapsed) {
            expandedRailWidth = clampRailWidth(storedRailWidth > 0 ? storedRailWidth : railWidth)
            sidebarCollapsed = false
            sidebarReveal = 1.0
            return
        }

        storedRailWidth = clampRailWidth(expandedRailWidth)
        sidebarCollapsed = true
        sidebarReveal = 0.0
    }

    function openQuickHelp(topicId, sourceItem) {
        clearHoverHint()
        const nextTopic = HelpCatalog.topic(topicId)
        if (!nextTopic || !nextTopic.title) {
            quickHelpTopicId = ""
            return
        }

        quickHelpTopicId = topicId
        detailHelpTopicId = ""

        let nextX = shellInset + HydraTheme.space12
        let nextY = shellInset + HydraTheme.space18
        if (sourceItem) {
            const point = sourceItem.mapToItem(root.contentItem, 0, sourceItem.height + HydraTheme.space8)
            nextX = point.x - quickHelpWidth + sourceItem.width
            nextY = point.y
        }

        quickHelpX = Math.max(shellInset + HydraTheme.space6,
                              Math.min(width - quickHelpWidth - shellInset - HydraTheme.space6, nextX))
        quickHelpY = Math.max(shellInset + HydraTheme.space6,
                              Math.min(height - 240, nextY))
    }

    function closeQuickHelp() {
        quickHelpTopicId = ""
    }

    function openDetailHelp(topicId) {
        clearHoverHint()
        const nextTopic = HelpCatalog.topic(topicId)
        if (!nextTopic || !nextTopic.title) {
            detailHelpTopicId = ""
            return
        }

        closeQuickHelp()
        detailHelpTopicId = topicId
    }

    function closeDetailHelp() {
        detailHelpTopicId = ""
    }

    function queueHoverHint(text, sourceItem) {
        if (!sourceItem || !text || text.length === 0) {
            clearHoverHint(sourceItem)
            return
        }
        if (quickHelpTopicId.length > 0 || detailHelpTopicId.length > 0) {
            clearHoverHint(sourceItem)
            return
        }
        hoverHintSource = sourceItem
        hoverHintText = ""
        hoverHintTimer.pendingText = text
        hoverHintTimer.restart()
    }

    function clearHoverHint(sourceItem) {
        if (sourceItem && hoverHintSource && sourceItem !== hoverHintSource) {
            return
        }
        hoverHintTimer.stop()
        hoverHintTimer.pendingText = ""
        hoverHintText = ""
        hoverHintSource = null
    }

    function showHoverHint(text, sourceItem) {
        if (!sourceItem || !text || text.length === 0) {
            clearHoverHint()
            return
        }

        const point = sourceItem.mapToItem(root.contentItem,
                                           sourceItem.width + HydraTheme.space8,
                                           -HydraTheme.space4)
        hoverHintSource = sourceItem
        hoverHintText = text
        hoverHintX = Math.max(shellInset + HydraTheme.space6,
                              Math.min(width - hoverHintWidth - shellInset - HydraTheme.space6, point.x))
        hoverHintY = Math.max(shellInset + HydraTheme.space6,
                              Math.min(height - 120, point.y))
    }

    Component.onCompleted: {
        syncRailWidthBounds()
        sidebarReveal = sidebarCollapsed ? 0.0 : 1.0
        if (startupDetailHelpTopic.length > 0) {
            Qt.callLater(function() {
                root.openDetailHelp(startupDetailHelpTopic)
            })
        } else if (startupQuickHelpTopic.length > 0) {
            Qt.callLater(function() {
                root.openQuickHelp(startupQuickHelpTopic, null)
            })
        }
    }

    onMinRailWidthChanged: syncRailWidthBounds()
    onMaxRailWidthChanged: syncRailWidthBounds()

    Behavior on sidebarReveal {
        enabled: !dividerHandle.dragActive
        NumberAnimation {
            duration: HydraTheme.motionSlow
            easing.type: Easing.InOutCubic
        }
    }

    ShellBackdrop {
        anchors.fill: parent
    }

    Timer {
        id: hoverHintTimer

        property string pendingText: ""

        interval: 1350
        repeat: false
        onTriggered: {
            if (!root.hoverHintSource || pendingText.length === 0) {
                root.clearHoverHint()
                return
            }
            root.showHoverHint(pendingText, root.hoverHintSource)
        }
    }

    Shortcut {
        sequence: "Ctrl+B"
        onActivated: root.toggleSidebar()
    }

    Item {
        id: shellSurface

        anchors.fill: parent
        anchors.margins: root.shellInset

        Item {
            id: railFrame

            x: 0
            y: 0
            width: root.railVisibleWidth
            height: parent.height
            clip: true
            z: 1

            Rectangle {
                anchors.fill: parent
                radius: root.frameRadius
                color: HydraTheme.railBg
                border.width: 1
                border.color: HydraTheme.borderDark
                clip: true
                opacity: root.sidebarReveal

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    height: root.tightShell ? 18 : 22
                    color: HydraTheme.withAlpha(HydraTheme.boardPanelMuted, 0.44)
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: root.tightShell ? 18 : 22
                    height: 1
                    color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.24)
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 2
                    color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.7)
                }

                FrameCorners {
                    anchors.fill: parent
                    lineColor: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.24)
                }

                Item {
                    id: railViewport

                    anchors.fill: parent
                    anchors.margins: root.panelInset
                    clip: true

                    Item {
                        id: railContent

                        width: Math.max(0, root.expandedRailWidth - (root.panelInset * 2))
                        height: parent.height
                        x: Math.round((root.sidebarReveal - 1.0) * HydraTheme.space16)
                        opacity: Math.max(0.0, Math.min(1.0, (root.sidebarReveal - 0.08) / 0.92))

                        LaunchSidebar {
                            anchors.fill: parent
                            appState: root.appState
                            layoutWidth: parent.width
                            helpHost: root
                        }
                    }
                }
            }
        }

        Item {
            id: dividerTrack

            x: root.railVisibleWidth
            y: 0
            width: root.dividerWidth
            height: parent.height
            z: 3

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: HydraTheme.withAlpha(HydraTheme.borderDark, 0.78)
            }

            DividerHandle {
                id: dividerHandle

                tightMode: root.tightShell
                sidebarCollapsed: root.sidebarCollapsed
                hoverHost: root
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: root.panelInset + HydraTheme.space8
                onToggleRequested: root.toggleSidebar()
                onDragStarted: {
                    root.dividerBaseWidth = root.sidebarCollapsed ? root.minRailWidth : root.expandedRailWidth
                    if (root.sidebarCollapsed) {
                        root.sidebarCollapsed = false
                        root.sidebarReveal = 1.0
                        root.expandedRailWidth = root.minRailWidth
                        root.storedRailWidth = root.minRailWidth
                    }
                }
                onDragMoved: deltaX => {
                    const nextWidth = root.clampRailWidth(root.dividerBaseWidth + deltaX)
                    root.expandedRailWidth = nextWidth
                    root.storedRailWidth = nextWidth
                }
                onDragFinished: {
                    root.dividerBaseWidth = root.expandedRailWidth
                }
            }
        }

        Rectangle {
            id: boardFrame

            anchors.left: dividerTrack.right
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            radius: root.frameRadius
            color: HydraTheme.boardBg
            border.width: 1
            border.color: HydraTheme.borderDark
            clip: true
            z: 1

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: root.tightShell ? 18 : 22
                color: HydraTheme.withAlpha(HydraTheme.boardPanelMuted, 0.42)
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: root.tightShell ? 18 : 22
                height: 1
                color: HydraTheme.withAlpha(HydraTheme.accentSteel, 0.22)
            }

            FrameCorners {
                anchors.fill: parent
                lineColor: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.22)
            }

            SessionBoard {
                anchors.fill: parent
                anchors.margins: root.panelInset
                appState: root.appState
                layoutWidth: Math.max(0, root.boardLayoutWidth - (root.panelInset * 2))
                helpHost: root
            }
        }
    }

    QuickHelpBubble {
        anchors.fill: parent
        z: 20
        topicData: root.quickHelpData
        topicId: root.quickHelpTopicId
        bubbleX: root.quickHelpX
        bubbleY: root.quickHelpY
        bubbleWidth: root.quickHelpWidth
        onDetailRequested: topicId => root.openDetailHelp(topicId)
        onDismissRequested: root.closeQuickHelp()
    }

    HoverHintBubble {
        anchors.fill: parent
        z: 20
        text: root.hoverHintText
        bubbleX: root.hoverHintX
        bubbleY: root.hoverHintY
        bubbleWidth: root.hoverHintWidth
    }

    DetailHelpPanel {
        anchors.fill: parent
        z: 21
        topicData: root.detailHelpData
        panelWidth: root.detailHelpWidth
        panelHeight: root.detailHelpHeight
        onCloseRequested: root.closeDetailHelp()
    }
}
