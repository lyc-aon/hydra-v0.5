pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts 6.5
import "../styles"

Item {
    id: root

    required property QtObject appState
    property real layoutWidth: width
    property QtObject helpHost: null
    readonly property bool canLaunch: appState.selectedRepoId.length > 0 && appState.tmuxAvailable
    readonly property bool canCreateWorktree: appState.selectedRepoId.length > 0 && appState.repositoryIsGit
    readonly property real effectiveWidth: layoutWidth > 0 ? layoutWidth : width
    readonly property bool compactMode: effectiveWidth < 300
    readonly property bool denseMode: effectiveWidth < 252
    readonly property bool tightMode: effectiveWidth < 230

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

            Item {
                width: parent.width
                height: HydraTheme.space2
            }

            ConsoleHeader {
                width: parent.width
                appState: root.appState
                helpHost: root.helpHost
                denseMode: root.denseMode
                tightMode: root.tightMode
            }

            Rectangle {
                width: parent.width
                height: 1
                color: HydraTheme.withAlpha(HydraTheme.accentSteel, 0.22)
            }

            LaunchBusPanel {
                width: parent.width
                appState: root.appState
                helpHost: root.helpHost
                denseMode: root.denseMode
                tightMode: root.tightMode
            }

            TargetMapPanel {
                width: parent.width
                appState: root.appState
                helpHost: root.helpHost
                denseMode: root.denseMode
                tightMode: root.tightMode
                canCreateWorktree: root.canCreateWorktree
            }

            ExecutePanel {
                width: parent.width
                appState: root.appState
                helpHost: root.helpHost
                denseMode: root.denseMode
                tightMode: root.tightMode
                canLaunch: root.canLaunch
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
