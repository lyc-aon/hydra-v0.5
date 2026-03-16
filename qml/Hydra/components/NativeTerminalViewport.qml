pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import QMLTermWidget 2.0
import Hydra.Backend 1.0
import "../styles"

FocusScope {
    id: root

    required property TerminalSurfaceController controller
    required property string tmuxSessionName
    required property string workingDirectory
    required property var activeSessionNames
    property bool sessionStartEnabled: true
    property string accessibleName: "Embedded terminal viewport"
    property bool denseMode: false
    property bool hasSelection: false
    property bool focused: false
    property int columns: 0
    property int rows: 0

    signal sessionStartRequested()
    signal terminalActivated()

    activeFocusOnTab: true

    readonly property bool active: tmuxSessionName.length > 0
    readonly property bool live: {
        const host = root.currentHost()
        if (root.currentIndex < 0 || host === null) {
            return false
        }
        const liveNames = root.normalizedSessionNames()
        return liveNames.indexOf(root.tmuxSessionName) >= 0
    }
    property int currentIndex: -1

    function normalizedSessionNames() {
        const source = root.activeSessionNames
        if (source === undefined || source === null) {
            return []
        }
        if (Array.isArray(source)) {
            return source
        }
        if (typeof source === "string") {
            const trimmed = source.trim()
            return trimmed.length > 0 ? [trimmed] : []
        }

        const normalized = []
        const length = Number(source.length)
        if (!Number.isFinite(length) || length <= 0) {
            return normalized
        }
        for (let index = 0; index < length; ++index) {
            const value = String(source[index]).trim()
            if (value.length > 0) {
                normalized.push(value)
            }
        }
        return normalized
    }

    function currentHost() {
        return root.currentIndex >= 0 ? root.hostAt(root.currentIndex) : null
    }

    function hostAt(index): var {
        return terminalRepeater.itemAt(index)
    }

    function sessionIndex(sessionName) {
        const key = sessionName.trim()
        if (key.length === 0) {
            return -1
        }

        for (let index = 0; index < terminalModel.count; ++index) {
            if (terminalModel.get(index).sessionName === key) {
                return index
            }
        }

        return -1
    }

    function ensureCachedSession(sessionName, workingDirectory) {
        const key = sessionName.trim()
        if (key.length === 0) {
            return -1
        }

        const existingIndex = root.sessionIndex(key)
        if (existingIndex >= 0) {
            if (terminalModel.get(existingIndex).workingDirectory !== workingDirectory) {
                terminalModel.setProperty(existingIndex, "workingDirectory", workingDirectory)
            }
            return existingIndex
        }

        terminalModel.append({
            "sessionName": key,
            "workingDirectory": workingDirectory
        })
        return terminalModel.count - 1
    }

    function pruneCachedSessions() {
        const liveNames = {}
        const sessionNames = root.normalizedSessionNames()
        for (let index = 0; index < sessionNames.length; ++index) {
            const key = String(sessionNames[index]).trim()
            if (key.length > 0) {
                liveNames[key] = true
            }
        }

        for (let index = terminalModel.count - 1; index >= 0; --index) {
            const sessionName = String(terminalModel.get(index).sessionName).trim()
            if (!liveNames[sessionName]) {
                terminalModel.remove(index, 1)
            }
        }

        root.currentIndex = root.sessionIndex(root.tmuxSessionName)
        if (root.currentIndex < 0) {
            root.hasSelection = false
            root.focused = false
            root.columns = 0
            root.rows = 0
        }
    }

    function syncHostState() {
        const host = root.currentHost()
        root.hasSelection = host !== null ? host.hasSelection : false
        root.focused = host !== null ? host.focused : false
        root.columns = host !== null ? host.columns : 0
        root.rows = host !== null ? host.rows : 0
    }

    function focusTerminal() {
        const host = root.currentHost()
        root.terminalActivated()
        root.focus = true
        root.forceActiveFocus(Qt.OtherFocusReason)
        for (let index = 0; index < terminalModel.count; ++index) {
            const candidate = root.hostAt(index)
            if (candidate && candidate !== host && candidate["releaseTerminalFocus"]) {
                candidate["releaseTerminalFocus"]()
            }
        }
        if (host && host["focusTerminal"]) {
            if (host["startSessionIfReady"]) {
                host["startSessionIfReady"]()
            }
            host["focusTerminal"](Qt.OtherFocusReason)
            Qt.callLater(function() {
                if (host.visible && host["focusTerminal"]) {
                    if (host["startSessionIfReady"]) {
                        host["startSessionIfReady"]()
                    }
                    host["focusTerminal"](Qt.OtherFocusReason)
                }
            })
        }
    }

    function releaseTerminalFocus() {
        const host = root.currentHost()
        root.focus = false
        root.focused = false
        if (host && host["releaseTerminalFocus"]) {
            host["releaseTerminalFocus"]()
        }
    }

    function attachCommandArgs(sessionName) {
        const key = sessionName.trim()
        if (key.length === 0) {
            return []
        }

        return [
            "TERM=xterm-256color",
            "tmux",
            "attach-session", "-t", key
        ]
    }

    function colorSchemeForTheme() {
        switch (HydraTheme.currentThemeId) {
        case "hermes_veil":
            return "HydraHermes"
        case "openai":
            return "HydraOpenAI"
        case "chatgpt":
            return "HydraChatGPT"
        case "claude_paper":
            return "HydraClaudePaper"
        case "claude_ink":
            return "HydraClaudeInk"
        case "eva":
            return "HydraEva"
        case "og_steam":
            return "HydraSteam"
        default:
            return "HydraSteam"
        }
    }

    function copySelection() {
        const host = root.currentHost()
        if (host && host.copySelection) {
            host.copySelection()
        }
    }

    function pasteClipboard() {
        root.controller.pasteClipboard()
    }

    function activateSession() {
        if (!root.active) {
            root.currentIndex = -1
            root.hasSelection = false
            root.focused = false
            root.columns = 0
            root.rows = 0
            return
        }

        root.currentIndex = root.ensureCachedSession(root.tmuxSessionName, root.workingDirectory)
        Qt.callLater(function() {
            root.syncHostState()
        })
    }

    onTmuxSessionNameChanged: activateSession()
    onWorkingDirectoryChanged: activateSession()
    onActiveSessionNamesChanged: pruneCachedSessions()
    onSessionStartEnabledChanged: {
        if (!sessionStartEnabled) {
            return
        }
        const host = root.currentHost()
        if (host && host.startSessionIfReady) {
            Qt.callLater(function() {
                if (root.sessionStartEnabled && host.visible) {
                    host.startSessionIfReady()
                }
            })
        }
    }
    onVisibleChanged: {
        if (!visible) {
            return
        }
        const host = root.currentHost()
        if (host && host.startSessionIfReady) {
            Qt.callLater(function() {
                if (root.visible && host.visible) {
                    host.startSessionIfReady()
                }
            })
        }
    }
    Component.onCompleted: activateSession()
    Accessible.role: Accessible.Pane
    Accessible.name: root.accessibleName

    ListModel {
        id: terminalModel
    }

    Component {
        id: terminalComponent

        FocusScope {
            id: host

            required property string sessionName
            required property string workingDirectory

            property bool focused: terminal.activeFocus
            property int columns: terminal.terminalSize !== undefined && terminal.terminalSize
                                  ? terminal.terminalSize.width
                                  : 0
            property int rows: terminal.terminalSize !== undefined && terminal.terminalSize
                               ? terminal.terminalSize.height
                               : 0
            property bool hasSelection: terminal.hasSelection
            property bool sessionStarting: false
            property alias terminalSessionRef: terminalSession
            property alias terminalRef: terminal
            property string contextSelectionText: ""
            property real contextMenuX: 0
            property real contextMenuY: 0

            anchors.fill: parent
            visible: root.visible && host.sessionName === root.tmuxSessionName
            enabled: visible
            z: visible ? 1 : 0
            activeFocusOnTab: true

            function startSessionIfReady() {
                if (!root.sessionStartEnabled) return
                if (!host.visible) return
                if (host.width <= 1 || host.height <= 1) {
                    return
                }
                if (host.sessionStarting || host.terminalSessionRef.hasActiveProcess) return
                if (!root.controller.prepareInteractiveAttach(host.sessionName)) {
                    return
                }
                host.sessionStarting = true
                host.terminalSessionRef.startShellProgram()
            }

            onWidthChanged: startSessionIfReady()
            onHeightChanged: startSessionIfReady()
            onVisibleChanged: {
                startSessionIfReady()
                if (visible && root.activeFocus) {
                    Qt.callLater(function() {
                        if (host.visible) {
                            host.focusTerminal(Qt.OtherFocusReason)
                        }
                    })
                }
            }
            Component.onCompleted: startSessionIfReady()

            function focusTerminal(reason) {
                const focusReason = reason === undefined ? Qt.OtherFocusReason : reason
                host.startSessionIfReady()
                host.focus = true
                terminal.focus = true
                host.forceActiveFocus(focusReason)
                terminal.forceActiveFocus(focusReason)
            }

            function releaseTerminalFocus() {
                host.focus = false
                terminal.focus = false
            }

            function copySelection() {
                terminal.copyClipboard()
            }

            function pasteClipboard() {
                host.focusTerminal(Qt.OtherFocusReason)
                terminal.pasteClipboard()
            }

            function selectAllText() {
                if (terminal.screenLines !== undefined && terminal.screenColumns !== undefined) {
                    terminal.setSelectionStart(0, 0)
                    terminal.setSelectionEnd(Math.max(0, terminal.lines - 1),
                                             Math.max(0, terminal.columns - 1))
                }
            }

            function openContextMenu(position) {
                host.focusTerminal(Qt.OtherFocusReason)
                host.contextSelectionText = terminal.selectedText ? String(terminal.selectedText) : ""
                if (position !== undefined && position !== null) {
                    host.contextMenuX = Number(position.x)
                    host.contextMenuY = Number(position.y)
                } else {
                    host.contextMenuX = Math.max(0, host.width * 0.5 - terminalContextMenu.width * 0.5)
                    host.contextMenuY = Math.max(0, host.height * 0.5 - terminalContextMenu.height * 0.5)
                }
                terminalContextMenu.close()
                terminalContextMenu.open()
            }

            QMLTermWidget {
                id: terminal

                anchors.fill: host
                activeFocusOnTab: true
                font.family: HydraTheme.terminalMonoFamily
                font.pointSize: root.denseMode ? 10 : 11
                colorScheme: root.colorSchemeForTheme()
                enableBold: true
                enableItalic: true
                antialiasText: false
                useFBORendering: false
                forceLocalMouseSelection: root.controller.providerKey === "opencode"
                property string hostWorkingDirectory: host.workingDirectory
                property string hostSessionName: host.sessionName
                onActiveFocusChanged: {
                    if (host.visible) {
                        root.syncHostState()
                        if (terminal.activeFocus) {
                            root.terminalActivated()
                        }
                    }
                }
                onCopyAvailable: function(available) {
                    if (host.visible) {
                        root.syncHostState()
                    }
                }
                onInputKeyTyped: HydraSounds.playTerminalKey()
                onConfigureRequest: function(position) {
                    host.openContextMenu(position)
                }
                onMouseSignal: function(button, column, line, eventType) {
                    if (!host.visible || !root.controller.active || eventType !== 0) {
                        return
                    }

                    if (root.controller.providerKey === "opencode") {
                        return
                    }

                    const lineDelta = button === 4 ? 5 : (button === 5 ? -5 : 0)
                    if (lineDelta === 0) {
                        return
                    }

                    host.focusTerminal(Qt.OtherFocusReason)
                    root.controller.scrollHistory(lineDelta)
                }

                session: QMLTermSession {
                    id: terminalSession

                    historySize: -1
                    initialWorkingDirectory: terminal.hostWorkingDirectory.length > 0
                                             ? terminal.hostWorkingDirectory
                                             : "/tmp"
                    shellProgram: "env"
                    shellProgramArgs: root.attachCommandArgs(terminal.hostSessionName)
                }
            }

            Popup {
                id: terminalContextMenu

                parent: host
                modal: false
                focus: true
                padding: HydraTheme.space6
                closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
                x: Math.max(0, Math.min(host.contextMenuX, host.width - width))
                y: Math.max(0, Math.min(host.contextMenuY, host.height - height))

                background: Rectangle {
                    radius: 0
                    color: HydraTheme.chromeGlass
                    border.width: 1
                    border.color: HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.45)
                }

                contentItem: Column {
                    spacing: HydraTheme.space4

                    Button {
                        text: "Copy"
                        enabled: host.contextSelectionText.length > 0
                        onClicked: function() {
                            root.controller.copyText(host.contextSelectionText)
                            terminalContextMenu.close()
                            host.focusTerminal(Qt.OtherFocusReason)
                        }
                    }

                    Button {
                        text: "Paste"
                        enabled: root.controller.active
                        onClicked: function() {
                            root.pasteClipboard()
                            terminalContextMenu.close()
                            host.focusTerminal(Qt.OtherFocusReason)
                        }
                    }

                    Button {
                        text: "Select All"
                        enabled: root.controller.active
                        onClicked: function() {
                            host.selectAllText()
                            terminalContextMenu.close()
                            host.focusTerminal(Qt.OtherFocusReason)
                        }
                    }
                }
            }

            QMLTermScrollbar {
                id: scrollbar
                terminal: terminal
                width: root.denseMode ? 10 : 12
                anchors.right: host.right
                anchors.top: host.top
                anchors.bottom: host.bottom

                Rectangle {
                    anchors.margins: 3
                    anchors.fill: scrollbar
                    color: HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.14)
                    radius: 0
                }
            }
        }
    }

    Item {
        id: terminalStack
        anchors.fill: parent

        Repeater {
            id: terminalRepeater
            model: terminalModel
            delegate: terminalComponent
        }
    }
}
