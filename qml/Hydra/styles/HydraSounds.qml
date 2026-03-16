pragma Singleton
import QtQuick 6.5

QtObject {
    signal bootRequested()
    signal clickRequested()
    signal hoverRequested()
    signal approvalRequested()
    signal completionRequested()
    signal warningRequested()
    signal splashRequested()
    signal terminalKeyRequested()

    function playBoot() {
        bootRequested()
    }

    function playClick() {
        clickRequested()
    }

    function playHover() {
        hoverRequested()
    }

    function playApproval() {
        approvalRequested()
    }

    function playCompletion() {
        completionRequested()
    }

    function playWarning() {
        warningRequested()
    }

    function playSplash() {
        splashRequested()
    }

    function playTerminalKey() {
        terminalKeyRequested()
    }
}
