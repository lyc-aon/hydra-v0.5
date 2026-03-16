pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import Hydra.Backend 1.0
import "../styles"

Item {
    id: root

    required property AppState appState
    required property ThemeState themeState
    property bool fullscreenActive: false
    property var helpHost: null
    property bool denseMode: false
    property bool tightMode: false
    signal fullscreenToggleRequested()

    readonly property int horizontalInset: root.tightMode ? HydraTheme.space8 : HydraTheme.space10
    readonly property int topInset: 0
    readonly property int titleGap: HydraTheme.space4
    readonly property int chipInset: root.tightMode ? HydraTheme.space4 : HydraTheme.space6

    function requestHelp(topicId, sourceItem) {
        if (helpHost && helpHost.openQuickHelp) {
            helpHost.openQuickHelp(topicId, sourceItem)
        }
    }

    implicitHeight: root.denseMode ? 80 : 96

    Item {
        id: contentFrame

        anchors.fill: parent
        anchors.leftMargin: root.horizontalInset
        anchors.rightMargin: root.horizontalInset
        anchors.topMargin: root.topInset
        anchors.bottomMargin: HydraTheme.space4

        RowLayout {
            id: topRow

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            spacing: HydraTheme.space10

            Item {
                id: titleStack

                Layout.fillWidth: true
                implicitWidth: titleText.implicitWidth
                implicitHeight: titleText.implicitHeight + 1

                Text {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.topMargin: 1
                    text: "HYDRA"
                    color: HydraTheme.withAlpha(HydraTheme.accentPhosphor, 0.1)
                    font.family: HydraTheme.displayFamily
                    font.pixelSize: 24
                    font.bold: true
                    font.letterSpacing: 1.8
                    scale: 1.01
                }

                Text {
                    id: titleText

                    anchors.left: parent.left
                    anchors.top: parent.top
                    text: "HYDRA"
                    color: HydraTheme.textOnDark
                    font.family: HydraTheme.displayFamily
                    font.pixelSize: 24
                    font.bold: true
                    font.letterSpacing: 1.8
                }
            }

            RowLayout {
                id: controlGroup

                spacing: root.tightMode ? HydraTheme.space4 : HydraTheme.space6
                Layout.alignment: Qt.AlignVCenter

                ThemeCycleButton {
                    themeState: root.themeState
                    hoverHost: root.helpHost
                    denseMode: root.denseMode
                    tightMode: root.tightMode
                    Layout.alignment: Qt.AlignVCenter
                }

                FullscreenToggleButton {
                    activeState: root.fullscreenActive
                    hoverHost: root.helpHost
                    Layout.alignment: Qt.AlignVCenter
                    Layout.rightMargin: HydraTheme.space4
                    onTriggered: root.fullscreenToggleRequested()
                }

                InfoDotButton {
                    topicId: "console-overview"
                    briefText: "Hydra tracks repository targets, launch settings, and detached tmux sessions from one shell."
                    accessibleLabel: "Explain console overview"
                    hoverHost: root.helpHost
                    Layout.alignment: Qt.AlignVCenter
                    onHelpRequested: (topicId, source) => root.requestHelp(topicId, source)
                }
            }
        }

        Column {
            id: infoStack

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: topRow.bottom
            anchors.topMargin: root.titleGap
            spacing: 0

            Text {
                text: "ORCHESTRATION CONSOLE // DETACHED SHELLS"
                color: HydraTheme.panelRule
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                font.bold: true
                opacity: 0.8
            }

            Item {
                width: 1
                height: HydraTheme.space8
            }

            RowLayout {
                x: root.chipInset
                width: Math.max(0, parent.width - root.chipInset)
                spacing: HydraTheme.space8

                StatusChip {
                    Layout.preferredWidth: root.tightMode ? 64 : 82
                    Layout.preferredHeight: implicitHeight
                    hoverHost: root.helpHost
                    toneColor: root.appState.tmuxAvailable ? HydraTheme.accentReady : HydraTheme.danger
                    textColor: HydraTheme.textOnDark
                    fillOpacity: 0.18
                    borderOpacity: 0.4
                    text: root.appState.tmuxAvailable ? "[TMUX]" : "[NO TMUX]"
                    toolTipText: "TMUX reports whether detached shell launch is available."
                }

                StatusChip {
                    Layout.preferredWidth: root.tightMode ? 54 : 74
                    hoverHost: root.helpHost
                    toneColor: HydraTheme.accentSteelBright
                    textColor: HydraTheme.textOnDark
                    fillOpacity: 0.12
                    borderOpacity: 0.28
                    text: root.tightMode ? "R:" + root.appState.repoCount : "REPOS " + root.appState.repoCount
                    toolTipText: "REPOS is the number of repositories currently registered in Hydra."
                }

                StatusChip {
                    Layout.preferredWidth: root.tightMode ? 54 : 96
                    hoverHost: root.helpHost
                    toneColor: HydraTheme.accentBronze
                    textColor: HydraTheme.textOnDark
                    fillOpacity: 0.18
                    borderOpacity: 0.34
                    text: root.tightMode ? "W:" + root.appState.worktreeCount : "WORKTREES " + root.appState.worktreeCount
                    toolTipText: "WORKTREES is the number of linked worktrees currently loaded for the selected repository."
                }

                Item {
                    Layout.fillWidth: true
                }
            }
        }
    }
}
