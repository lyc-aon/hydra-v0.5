pragma ComponentBehavior: Bound
import QtQuick 6.5
import Hydra.Backend 1.0
import "../styles"

FocusScope {
    id: root

    property AppState appState: null
    property ThemeState themeState: null
    property DesktopDialogBridge desktopBridge: null
    property bool fullscreenActive: false
    property real layoutWidth: width
    property var helpHost: null
    property var contentProxy: contentLoader.item ? contentLoader.item : null
    signal toggleFullscreenRequested()
    readonly property bool backendReady: root.appState && root.themeState && root.desktopBridge
    readonly property bool canLaunch: root.backendReady
                                      && (appState.launchInHomeDirectory
                                          ? appState.repoCount > 0
                                          : appState.selectedRepoId.length > 0)
                                      && appState.tmuxAvailable
                                      && appState.selectedProviderAvailable
    readonly property bool canCreateWorktree: root.backendReady
                                              && appState.selectedRepoId.length > 0
                                              && appState.repositoryIsGit
    readonly property real effectiveWidth: layoutWidth > 0 ? layoutWidth : width
    readonly property bool compactMode: effectiveWidth < 300
    readonly property bool denseMode: effectiveWidth < 252
    readonly property bool tightMode: effectiveWidth < 230
    readonly property bool focusWithin: root.activeFocus
    readonly property var sectionSignalAnchors: {
        return root.contentProxy ? root.contentProxy.sectionSignalAnchors : []
    }

    activeFocusOnTab: true

    function focusPrimaryControl() {
        if (root.contentProxy && root.contentProxy.focusPrimaryControl) {
            root.contentProxy.focusPrimaryControl()
            return
        }
        root.forceActiveFocus()
    }

    Loader {
        id: contentLoader

        anchors.fill: parent
        active: root.backendReady

        sourceComponent: Item {
            anchors.fill: parent

            readonly property var sectionSignalAnchors: {
                const anchors = [
                    executePanel.y - scrollArea.contentY + HydraTheme.space12,
                    targetMapPanel.y - scrollArea.contentY + HydraTheme.space12
                ]
                if (resumePanel.visible) {
                    anchors.push(resumePanel.y - scrollArea.contentY + HydraTheme.space12)
                }
                return anchors
            }

            function focusPrimaryControl() {
                if (executePanel && executePanel.focusPrimaryControl) {
                    executePanel.focusPrimaryControl()
                }
            }

            Flickable {
                id: scrollArea

                anchors.fill: parent
                contentWidth: width
                contentHeight: sidebarContent.implicitHeight
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                flickableDirection: Flickable.VerticalFlick
                interactive: contentHeight > height

                Column {
                    id: sidebarContent

                    width: scrollArea.width - (scrollBar.visible ? scrollBar.width + HydraTheme.space8 : 0)
                    spacing: root.denseMode ? HydraTheme.space8 : HydraTheme.space10

                    ConsoleHeader {
                        width: parent.width
                        appState: root.appState
                        themeState: root.themeState
                        fullscreenActive: root.fullscreenActive
                        helpHost: root.helpHost
                        denseMode: root.denseMode
                        tightMode: root.tightMode
                        onFullscreenToggleRequested: root.toggleFullscreenRequested()
                    }

                    Rectangle {
                        width: parent.width
                        height: 1
                        color: HydraTheme.withAlpha(HydraTheme.accentSteel, 0.22)
                    }

                    ExecutePanel {
                        id: executePanel
                        width: parent.width
                        appState: root.appState
                        helpHost: root.helpHost
                        denseMode: root.denseMode
                        tightMode: root.tightMode
                        canLaunch: root.canLaunch
                    }

                    TargetMapPanel {
                        id: targetMapPanel
                        width: parent.width
                        appState: root.appState
                        desktopBridge: root.desktopBridge
                        helpHost: root.helpHost
                        denseMode: root.denseMode
                        tightMode: root.tightMode
                        canCreateWorktree: root.canCreateWorktree
                    }

                    ResumePanel {
                        id: resumePanel
                        width: parent.width
                        appState: root.appState
                        helpHost: root.helpHost
                        denseMode: root.denseMode
                        tightMode: root.tightMode
                    }

                    Item {
                        width: parent.width
                        height: HydraTheme.space16
                    }
                }
            }

            Rectangle {
                id: scrollBar

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                width: 3
                radius: 2
                color: HydraTheme.withAlpha(HydraTheme.textOnDark, 0.08)
                visible: scrollArea.visibleArea.heightRatio < 1.0

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    y: scrollArea.visibleArea.yPosition * parent.height
                    height: Math.max(36, scrollArea.visibleArea.heightRatio * parent.height)
                    radius: 2
                    color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.62)
                }
            }
        }
    }
}
