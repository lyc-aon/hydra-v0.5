pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Window 6.5
import Hydra.Backend 1.0
import "HelpCatalog.js" as HelpCatalog
import "components"
import "styles"

Window {
    id: root

    property AppState appState: null
    property MasterState masterState: null
    property RouterState routerState: null
    property ShellState shellState: null
    property TerminalSurfaceController terminalController: null
    property TerminalSurfaceController masterTerminalController: null
    property TerminalSurfaceController routerTerminalController: null
    property ThemeState themeState: null
    property DesktopDialogBridge desktopBridge: null
    property bool startupSidebarCollapsed: false
    property bool startupSidebarCollapsedSet: false
    property int startupSidebarWidth: -1
    property string startupQuickHelpTopic: ""
    property string startupDetailHelpTopic: ""
    property string startupSessionTraceName: ""
    property bool startupSkipBoot: false
    property bool startupShellReady: startupSkipBoot
    property bool startupLifecycleStarted: false
    readonly property bool startupOverlayActive: !startupShellReady
    readonly property bool shellShortcutsEnabled: startupShellReady
    property string activeViewMode: shellState ? shellState.activeViewMode : "workbench"
    property bool masterViewLoaded: false
    property bool nousSplashActive: !startupSkipBoot
    property bool bootScreenActive: false
    property bool closeAfterShutdownApproval: false
    readonly property bool initialSidebarCollapsed: startupSidebarCollapsedSet
                                                    ? startupSidebarCollapsed
                                                    : (shellState ? shellState.sidebarCollapsed : false)
    readonly property int initialSidebarWidth: startupSidebarWidth > 0
                                               ? startupSidebarWidth
                                               : (shellState && shellState.sidebarWidth > 0
                                                      ? shellState.sidebarWidth
                                                      : railWidth)
    property bool sidebarCollapsed: initialSidebarCollapsed
    property real sidebarReveal: initialSidebarCollapsed ? 0.0 : 1.0
    property real dividerBaseWidth: 0
    property real expandedRailWidth: Math.max(minRailWidth,
                                              Math.min(maxRailWidth,
                                                       initialSidebarWidth))
    property real storedRailWidth: expandedRailWidth
    property string quickHelpTopicId: ""
    property string detailHelpTopicId: ""
    property string hoverHintText: ""
    property Item hoverHintSource: null
    property real quickHelpX: 0
    property real quickHelpY: 0
    property real hoverHintX: 0
    property real hoverHintY: 0
    property var sidebarProxy: shellSurface ? shellSurface.sidebarItem : null
    readonly property bool compactShell: width < 1420
    readonly property bool narrowShell: width < 1220
    readonly property bool tightShell: width < 1080
    readonly property int railWidth: tightShell ? 248 : (narrowShell ? 276 : (compactShell ? 308 : 356))
    readonly property int minRailWidth: tightShell ? 208 : (narrowShell ? 228 : (compactShell ? 256 : 288))
    readonly property int maxRailWidth: tightShell ? 300 : (narrowShell ? 344 : (compactShell ? 392 : 452))
    readonly property int dividerWidth: tightShell ? 16 : 18
    readonly property int paneSeamGap: tightShell ? 4 : 6
    readonly property int shellInset: 0
    readonly property int panelInset: tightShell ? HydraTheme.space10 : HydraTheme.space12
    readonly property int frameRadius: tightShell ? HydraTheme.radius10 : HydraTheme.radius12
    readonly property int quickHelpWidth: tightShell ? 272 : 320
    readonly property int detailHelpWidth: Math.min(720, width - (shellInset * 2) - HydraTheme.space32)
    readonly property int detailHelpHeight: Math.min(640, height - (shellInset * 2) - HydraTheme.space32)
    readonly property int hoverHintWidth: tightShell ? 212 : 248
    readonly property real railVisibleWidth: Math.round(expandedRailWidth * sidebarReveal)
    readonly property real dividerCenterX: railVisibleWidth + (paneSeamGap * 0.5)
    readonly property real boardX: railVisibleWidth + paneSeamGap
    readonly property real boardLayoutWidth: Math.max(0, width - (shellInset * 2) - boardX)
    readonly property bool fullscreenActive: root.visibility === Window.FullScreen
    readonly property var quickHelpData: HelpCatalog.topic(quickHelpTopicId)
    readonly property var detailHelpData: HelpCatalog.topic(detailHelpTopicId)
    readonly property var workbenchPane: shellSurface ? shellSurface.workbenchItem : null
    readonly property var masterPane: shellSurface ? shellSurface.masterItem : null
    readonly property bool sidebarBackendReady: Boolean(root.appState && root.themeState && root.desktopBridge)
    readonly property bool workbenchBackendReady: Boolean(root.appState
                                                          && root.shellState
                                                          && root.terminalController)
    readonly property bool masterBackendReady: Boolean(root.appState
                                                       && root.masterState
                                                       && root.routerState
                                                       && root.shellState
                                                       && root.masterTerminalController
                                                       && root.routerTerminalController)

    width: 1460
    height: 920
    minimumWidth: 960
    minimumHeight: 680
    visible: true
    title: "Hydra V2"
    color: HydraTheme.shellBg

    onClosing: function(close) {
        const ownedLiveCount = root.appState ? root.appState.ownedLiveSessionCount : 0
        if (!root.closeAfterShutdownApproval && ownedLiveCount > 0) {
            close.accepted = false
            closeConfirmDialogHost.openDialog()
            HydraSounds.playWarning()
            return
        }

        if (!root.closeAfterShutdownApproval) {
            return
        }

        root.closeAfterShutdownApproval = false
    }

    function clampRailWidth(value) {
        return Math.max(minRailWidth, Math.min(maxRailWidth, value))
    }

    function applyTheme(themeId) {
        if (!themeId || themeId.length === 0) {
            return
        }
        HydraTheme.setTheme(themeId)
    }

    function setBorderlessFullscreen(enabled) {
        const nextVisibility = enabled ? Window.FullScreen : Window.Windowed
        if (root.visibility === nextVisibility) {
            return
        }
        root.visibility = nextVisibility
        shellState.fullscreen = enabled
    }

    function toggleBorderlessFullscreen() {
        setBorderlessFullscreen(!fullscreenActive)
    }

    function syncRailWidthBounds() {
        const preferredWidth = initialSidebarWidth > 0 ? initialSidebarWidth : railWidth
        const baseWidth = clampRailWidth(preferredWidth)
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
            shellState.sidebarCollapsed = false
            shellState.sidebarWidth = Math.round(clampRailWidth(expandedRailWidth))
            return
        }

        storedRailWidth = clampRailWidth(expandedRailWidth)
        sidebarCollapsed = true
        sidebarReveal = 0.0
        shellState.sidebarCollapsed = true
        shellState.sidebarWidth = Math.round(storedRailWidth)
    }

    function toggleViewMode() {
        const nextMode = root.activeViewMode === "workbench" ? "master" : "workbench"
        root.activeViewMode = nextMode
    }

    function finishStartupSequence() {
        if (root.startupShellReady) {
            return
        }

        if (startupOverlays.nousSplashItem && startupOverlays.nousSplashItem.abort) {
            startupOverlays.nousSplashItem.abort()
        }
        if (startupOverlays.bootScreenItem && startupOverlays.bootScreenItem.abort) {
            startupOverlays.bootScreenItem.abort()
        }

        root.nousSplashActive = false
        root.bootScreenActive = false
        root.startupShellReady = true
    }

    function skipStartupSequence() {
        finishStartupSequence()
    }

    function ensureStartupLifecycle() {
        if (root.startupLifecycleStarted || !root.appState) {
            return
        }

        root.startupLifecycleStarted = true
        Qt.callLater(function() {
            root.appState.startLifecycle()
        })
    }

    function focusRailPane() {
        if (sidebarCollapsed) {
            toggleSidebar()
            Qt.callLater(function() {
                if (sidebarProxy && sidebarProxy.focusPrimaryControl) {
                    sidebarProxy.focusPrimaryControl()
                }
            })
            return
        }
        if (sidebarProxy && sidebarProxy.focusPrimaryControl) {
            sidebarProxy.focusPrimaryControl()
        }
    }

    function focusSessionPane() {
        if (root.activeViewMode === "master") {
            if (root.masterPane) {
                root.masterPane.focusOrbitStrip()
            }
        } else {
            if (root.workbenchPane) {
                root.workbenchPane.focusSessionBoard()
            }
        }
    }

    function focusTerminalPane() {
        if (root.activeViewMode === "master") {
            if (root.masterPane) {
                root.masterPane.focusMasterTerminal()
            }
        } else {
            if (root.workbenchPane) {
                root.workbenchPane.focusTerminalPanel()
            }
        }
    }

    function focusRouterPane() {
        if (root.activeViewMode === "master" && root.masterPane) {
            root.masterPane.focusRouterPane()
        }
    }

    function currentFocusZone() {
        if (detailHelpTopicId.length > 0 || quickHelpTopicId.length > 0) {
            return "help"
        }
        if (root.activeViewMode === "master") {
            if (root.masterPane && root.masterPane.terminalSurfaceFocused) {
                return "terminal"
            }
            if (root.masterPane && root.masterPane.routerPanelFocused) {
                return "router"
            }
            if (root.masterPane && root.masterPane.orbitStripFocused) {
                return "sessions"
            }
            if (sidebarProxy && sidebarProxy.focusWithin) {
                return "rail"
            }
            return sidebarCollapsed ? "sessions" : "rail"
        }
        if (root.workbenchPane && root.workbenchPane.terminalPanelFocused) {
            return "terminal"
        }
        if (root.workbenchPane && root.workbenchPane.sessionBoardFocused) {
            return "sessions"
        }
        if (sidebarProxy && sidebarProxy.focusWithin) {
            return "rail"
        }
        return sidebarCollapsed ? "sessions" : "rail"
    }

    function cycleFocusZone(reverse) {
        const zones = root.activeViewMode === "master"
                      ? (sidebarCollapsed
                             ? ["sessions", "terminal", "router"]
                             : ["rail", "sessions", "terminal", "router"])
                      : (sidebarCollapsed ? ["sessions", "terminal"] : ["rail", "sessions", "terminal"])
        const current = currentFocusZone()
        let currentIndex = zones.indexOf(current)
        if (currentIndex < 0) {
            currentIndex = 0
        }
        const delta = reverse ? -1 : 1
        const nextIndex = (currentIndex + delta + zones.length) % zones.length
        switch (zones[nextIndex]) {
        case "rail":
            focusRailPane()
            break
        case "router":
            focusRouterPane()
            break
        case "terminal":
            focusTerminalPane()
            break
        case "sessions":
        default:
            focusSessionPane()
            break
        }
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
        if (themeState) {
            root.applyTheme(themeState.currentThemeId)
        }
        syncRailWidthBounds()
        sidebarReveal = sidebarCollapsed ? 0.0 : 1.0
        root.masterViewLoaded = root.activeViewMode === "master"
        root.ensureStartupLifecycle()
        if (root.startupSkipBoot) {
            root.nousSplashActive = false
            root.bootScreenActive = false
            root.startupShellReady = true
        }
        if (shellState && shellState.fullscreen) {
            Qt.callLater(function() {
                root.setBorderlessFullscreen(true)
            })
        }
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

    onActiveViewModeChanged: {
        if (root.shellState.activeViewMode !== root.activeViewMode) {
            root.shellState.activeViewMode = root.activeViewMode
        }
        if (root.activeViewMode === "master") {
            root.masterViewLoaded = true
        }
        Qt.callLater(function() {
            if (root.activeViewMode === "master") {
                root.focusSessionPane()
            } else {
                root.focusSessionPane()
            }
        })
    }

    Component.onDestruction: {
        if (!shellState) {
            return
        }
        shellState.sidebarCollapsed = sidebarCollapsed
        shellState.sidebarWidth = Math.round(clampRailWidth(sidebarCollapsed ? storedRailWidth : expandedRailWidth))
        shellState.fullscreen = fullscreenActive
    }

    onMinRailWidthChanged: syncRailWidthBounds()
    onMaxRailWidthChanged: syncRailWidthBounds()

    Connections {
        target: HydraSounds

        function onBootRequested() {
            if (root.desktopBridge) {
                root.desktopBridge.playBootSound()
            }
        }

        function onClickRequested() {
            if (root.desktopBridge) {
                root.desktopBridge.playClickSound()
            }
        }

        function onHoverRequested() {
            if (root.desktopBridge) {
                root.desktopBridge.playHoverSound()
            }
        }

        function onApprovalRequested() {
            if (root.desktopBridge) {
                root.desktopBridge.playApprovalSound()
            }
        }

        function onCompletionRequested() {
            if (root.desktopBridge) {
                root.desktopBridge.playCompletionSound()
            }
        }

        function onWarningRequested() {
            if (root.desktopBridge) {
                root.desktopBridge.playWarningSound()
            }
        }

        function onSplashRequested() {
            if (root.desktopBridge) {
                root.desktopBridge.playSplashSound()
            }
        }

        function onTerminalKeyRequested() {
            if (root.desktopBridge) {
                root.desktopBridge.playTerminalKeySound()
            }
        }
    }

    Connections {
        target: root.themeState

        function onCurrentThemeIdChanged() {
            root.applyTheme(root.themeState.currentThemeId)
        }
    }

    Behavior on sidebarReveal {
        enabled: !(shellSurface && shellSurface.dividerDragActive)
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

    AppShortcutHub {
        host: root
    }

    AppShellSurface {
        id: shellSurface

        anchors.fill: parent
        anchors.margins: root.shellInset
        appState: root.appState
        masterState: root.masterState
        routerState: root.routerState
        shellState: root.shellState
        themeState: root.themeState
        desktopBridge: root.desktopBridge
        terminalController: root.terminalController
        masterTerminalController: root.masterTerminalController
        routerTerminalController: root.routerTerminalController
        helpHost: root
        startupShellReady: root.startupShellReady
        sidebarBackendReady: root.sidebarBackendReady
        workbenchBackendReady: root.workbenchBackendReady
        masterBackendReady: root.masterBackendReady
        sidebarCollapsed: root.sidebarCollapsed
        sidebarReveal: root.sidebarReveal
        panelInset: root.panelInset
        expandedRailWidth: root.expandedRailWidth
        railVisibleWidth: root.railVisibleWidth
        dividerCenterX: root.dividerCenterX
        boardX: root.boardX
        dividerWidth: root.dividerWidth
        frameRadius: root.frameRadius
        tightShell: root.tightShell
        fullscreenActive: root.fullscreenActive
        masterViewLoaded: root.masterViewLoaded
        activeViewMode: root.activeViewMode
        startupExpandedSessionName: root.startupSessionTraceName
        onToggleSidebarRequested: root.toggleSidebar()
        onDividerDragStarted: {
            root.dividerBaseWidth = root.sidebarCollapsed ? root.minRailWidth : root.expandedRailWidth
            if (root.sidebarCollapsed) {
                root.sidebarCollapsed = false
                root.sidebarReveal = 1.0
                root.expandedRailWidth = root.minRailWidth
                root.storedRailWidth = root.minRailWidth
                root.shellState.sidebarCollapsed = false
                root.shellState.sidebarWidth = Math.round(root.minRailWidth)
            }
        }
        onDividerDragMoved: deltaX => {
            const nextWidth = root.clampRailWidth(root.dividerBaseWidth + deltaX)
            root.expandedRailWidth = nextWidth
            root.storedRailWidth = nextWidth
            root.shellState.sidebarWidth = Math.round(nextWidth)
        }
        onDividerDragFinished: {
            root.dividerBaseWidth = root.expandedRailWidth
            root.shellState.sidebarWidth = Math.round(root.clampRailWidth(root.expandedRailWidth))
        }
        onActivateWorkbenchRequested: root.activeViewMode = "workbench"
        onActivateMasterRequested: root.activeViewMode = "master"
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

    CloseConfirmDialog {
        id: closeConfirmDialogHost
        host: root
    }

    StartupOverlayStack {
        id: startupOverlays
        anchors.fill: parent
        startupOverlayActive: root.startupOverlayActive
        startupSkipBoot: root.startupSkipBoot
        startupShellReady: root.startupShellReady
        bootScreenActive: root.bootScreenActive
        nousSplashActive: root.nousSplashActive
        onSkipRequested: root.skipStartupSequence()
        onBootDismissed: {
            root.bootScreenActive = false
            root.finishStartupSequence()
        }
        onSplashDismissed: {
            root.nousSplashActive = false
            root.bootScreenActive = true
            startupOverlays.bootScreenItem.start()
        }
    }
}
