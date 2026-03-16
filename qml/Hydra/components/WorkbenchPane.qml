pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import Hydra.Backend 1.0
import "../styles"

Item {
    id: root

    required property AppState appState
    required property ShellState shellState
    required property TerminalSurfaceController terminalController
    property var helpHost: null
    property string startupExpandedSessionName: ""
    readonly property bool stackedMode: width < 1240
    readonly property bool sessionBoardFocused: Boolean(sessionBoard && sessionBoard.focusWithin)
    readonly property bool terminalPanelFocused: Boolean(terminalPanel && terminalPanel.focusWithin)
    signal sessionActivated(string sessionId)

    function clampWideFraction(value) {
        return Math.max(0.28, Math.min(0.72, value))
    }

    function clampStackedFraction(value) {
        return Math.max(0.30, Math.min(0.70, value))
    }

    function persistSplitFraction() {
        if (root.stackedMode) {
            if (splitView.height <= 0 || sessionBoard.height <= 0) {
                return
            }
            root.shellState.stackedSessionBoardFraction =
                    root.clampStackedFraction(sessionBoard.height / splitView.height)
            return
        }

        if (splitView.width <= 0 || sessionBoard.width <= 0) {
            return
        }
        root.shellState.wideSessionBoardFraction =
                root.clampWideFraction(sessionBoard.width / splitView.width)
    }

    function focusSessionBoard() {
        sessionBoard.focusBoard()
    }

    function focusTerminalPanel() {
        terminalPanel.focusTerminalPanel()
    }

    function toggleSelectedSessionTrace() {
        sessionBoard.toggleSelectedTrace()
    }

    function terminateSelectedSession() {
        sessionBoard.terminateSelectedSession()
    }

    SplitView {
        id: splitView

        anchors.fill: parent
        orientation: root.stackedMode ? Qt.Vertical : Qt.Horizontal
        handle: Rectangle {
            implicitWidth: root.stackedMode ? splitView.width : 12
            implicitHeight: root.stackedMode ? 12 : splitView.height
            color: "transparent"

            Rectangle {
                anchors.centerIn: parent
                width: root.stackedMode ? parent.width : 1
                height: root.stackedMode ? 1 : parent.height
                color: HydraTheme.withAlpha(HydraTheme.borderDark, 0.34)
            }
        }

        SessionBoard {
            id: sessionBoard
            SplitView.fillWidth: true
            SplitView.fillHeight: true
            SplitView.preferredWidth: root.stackedMode
                                      ? width
                                      : Math.round(splitView.width
                                                   * root.clampWideFraction(root.shellState.wideSessionBoardFraction))
            SplitView.preferredHeight: root.stackedMode
                                       ? Math.round(splitView.height
                                                    * root.clampStackedFraction(root.shellState.stackedSessionBoardFraction))
                                       : height
            SplitView.minimumWidth: 340
            SplitView.minimumHeight: 260
            appState: root.appState
            showLaunchSafetyChips: true
            layoutWidth: width
            helpHost: root.helpHost
            startupExpandedSessionName: root.startupExpandedSessionName

            onWidthChanged: Qt.callLater(root.persistSplitFraction)
            onHeightChanged: Qt.callLater(root.persistSplitFraction)
            onSessionActivated: sessionId => {
                root.sessionActivated(sessionId)
                Qt.callLater(function() {
                    terminalPanel.focusTerminalPanel()
                })
            }
        }

        TerminalSurfacePanel {
            id: terminalPanel
            SplitView.fillWidth: true
            SplitView.fillHeight: true
            SplitView.preferredWidth: root.stackedMode ? width : Math.round(splitView.width * 0.58)
            SplitView.preferredHeight: root.stackedMode ? Math.round(splitView.height * 0.55) : height
            SplitView.minimumWidth: 420
            SplitView.minimumHeight: 260
            appState: root.appState
            terminalController: root.terminalController
            showLaunchSafetyChip: true
            helpHost: root.helpHost
        }
    }
}
