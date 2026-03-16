pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import Hydra.Backend 1.0
import "../styles"

Rectangle {
    id: root

    required property AppState appState
    property var hoverHost: null
    property bool showLaunchSafetyChips: true
    property string startupExpandedSessionName: ""
    property bool compactMode: false
    property bool denseMode: false
    property bool narrowMode: false
    property int boardInset: HydraTheme.space12
    property real refreshCascadeProgress: -0.26
    property bool refreshCascadeActive: false
    property string expandedSessionId: ""
    property real preservedSessionListContentY: -1
    readonly property int gridColumnCount: Math.max(4, Math.ceil(root.width / (root.narrowMode ? 112 : 136)))
    readonly property int gridRowCount: Math.max(4, Math.ceil(root.height / (root.denseMode ? 82 : 96)))
    readonly property bool evaTheme: HydraTheme.currentThemeId === "eva"
    readonly property var refreshLineOffsets: root.denseMode ? [0, 8, 16, 24, 34] : [0, 10, 20, 32, 46]
    readonly property real refreshLeadY: root.refreshCascadeProgress * Math.max(root.height, 1)
    signal sessionActivated(string sessionId)
    signal aliasEditRequested(string sessionId, string currentAlias)

    activeFocusOnTab: true

    function ensureSelection() {
        if (root.appState.selectedSessionId.length > 0 || root.appState.sessionCount <= 0) {
            return
        }
        root.appState.selectFirstSession()
    }

    function focusBoard() {
        ensureSelection()
        root.forceActiveFocus()
    }

    function moveSelection(delta) {
        if (root.appState.sessionCount <= 0) {
            return
        }
        if (!root.appState.selectAdjacentSession(delta)) {
            return
        }
        const model = sessionList.model
        if (!model || !model.indexOfSessionId) {
            return
        }
        const nextIndex = root.appState.sessionIndexOfId(root.appState.selectedSessionId)
        if (nextIndex >= 0) {
            sessionList.positionViewAtIndex(nextIndex, ListView.Contain)
        }
    }

    function selectNextSession() {
        moveSelection(1)
    }

    function selectPreviousSession() {
        moveSelection(-1)
    }

    function toggleSelectedTrace() {
        ensureSelection()
        const sessionId = root.appState.selectedSessionId
        if (sessionId.length === 0) {
            return
        }
        if (root.expandedSessionId === sessionId) {
            root.expandedSessionId = ""
            root.startupExpandedSessionName = ""
            return
        }
        root.expandedSessionId = sessionId
        root.startupExpandedSessionName = ""
    }

    function terminateSelectedSession() {
        ensureSelection()
        if (root.appState.selectedSessionId.length === 0) {
            return
        }
        root.appState.terminateSession(root.appState.selectedSessionId)
    }

    color: HydraTheme.boardPanel
    border.width: 1
    border.color: HydraTheme.borderDark
    radius: root.denseMode ? HydraTheme.radius8 : HydraTheme.radius10
    clip: true

    Keys.priority: Keys.BeforeItem
    Keys.onPressed: event => {
        if (event.key === Qt.Key_Down || event.key === Qt.Key_J) {
            root.selectNextSession()
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Up || event.key === Qt.Key_K) {
            root.selectPreviousSession()
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Home) {
            const model = sessionList.model
            if (root.appState.selectFirstSession() && model && model.indexOfSessionId) {
                const index = root.appState.sessionIndexOfId(root.appState.selectedSessionId)
                if (index >= 0) {
                    sessionList.positionViewAtIndex(index, ListView.Beginning)
                }
            }
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_End) {
            const model = sessionList.model
            if (root.appState.selectLastSession() && model && model.indexOfSessionId) {
                const index = root.appState.sessionIndexOfId(root.appState.selectedSessionId)
                if (index >= 0) {
                    sessionList.positionViewAtIndex(index, ListView.End)
                }
            }
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter
                || event.key === Qt.Key_Space || event.key === Qt.Key_T) {
            root.toggleSelectedTrace()
            event.accepted = true
            return
        }
        if ((event.key === Qt.Key_Delete && (event.modifiers & Qt.ShiftModifier))
                || (event.key === Qt.Key_X && (event.modifiers & Qt.ShiftModifier))) {
            root.terminateSelectedSession()
            event.accepted = true
        }
    }

    Connections {
        target: root.appState

        function onActivityPulseChanged() {
            if (root.appState.lastActivityKind === "refresh") {
                refreshCascade.restart()
            }
        }
    }

    Item {
        anchors.fill: parent

        Repeater {
            model: root.gridColumnCount

            Rectangle {
                required property int index

                x: Math.round(((index + 1) * root.width) / (root.gridColumnCount + 1))
                y: 0
                width: 1
                height: root.height
                color: HydraTheme.withAlpha(HydraTheme.gridLine, index % 3 === 0 ? 0.15 : 0.09)
            }
        }

        Repeater {
            model: root.gridRowCount

            Rectangle {
                required property int index

                x: 0
                y: Math.round(((index + 1) * root.height) / (root.gridRowCount + 1))
                width: root.width
                height: 1
                color: HydraTheme.withAlpha(HydraTheme.gridLine, index % 2 === 0 ? 0.13 : 0.08)
            }
        }
    }

    Item {
        anchors.fill: parent
        visible: root.refreshCascadeActive

        Rectangle {
            width: parent.width
            height: root.denseMode ? 58 : 72
            y: Math.round(root.refreshLeadY - (height * 0.55))
            visible: root.refreshCascadeActive
            color: "transparent"
            opacity: root.evaTheme ? 0.16 : 0.56
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop { position: 0.0; color: "transparent" }
                GradientStop { position: 0.22; color: HydraTheme.withAlpha(HydraTheme.panelRule, root.evaTheme ? 0.01 : 0.04) }
                GradientStop { position: 0.48; color: HydraTheme.withAlpha(HydraTheme.panelRule, root.evaTheme ? 0.04 : 0.18) }
                GradientStop { position: 0.72; color: HydraTheme.withAlpha(HydraTheme.accentPhosphorSoft, root.evaTheme ? 0.02 : 0.08) }
                GradientStop { position: 1.0; color: "transparent" }
            }
        }

        Repeater {
            model: root.refreshLineOffsets.length

            Rectangle {
                required property int index

                readonly property real lineOffset: root.refreshLineOffsets[index]
                x: 0
                y: Math.round(root.refreshLeadY - lineOffset)
                width: parent ? parent.width : 0
                height: index === 0 ? 2 : 1
                radius: height
                visible: root.refreshCascadeActive
                color: index < 2
                       ? HydraTheme.withAlpha(HydraTheme.accentPhosphorSoft, 0.58 - (index * 0.14))
                       : HydraTheme.withAlpha(HydraTheme.panelRule, 0.34 - ((index - 2) * 0.08))
                opacity: 0.96 - (index * 0.14)
            }
        }
    }

    SequentialAnimation {
        id: refreshCascade

        running: false

        ScriptAction {
            script: {
                root.refreshCascadeProgress = -0.26
                root.refreshCascadeActive = true
            }
        }

        NumberAnimation {
            target: root
            property: "refreshCascadeProgress"
            from: -0.26
            to: 1.2
            duration: 760
            easing.type: Easing.InOutCubic
        }

        ScriptAction {
            script: {
                root.refreshCascadeActive = false
                root.refreshCascadeProgress = -0.26
            }
        }
    }

    FrameCorners {
        anchors.fill: parent
        lineColor: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.22)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.boardInset
        spacing: root.denseMode ? HydraTheme.space8 : HydraTheme.space10

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Column {
                anchors.left: parent.left
                anchors.top: parent.top
                width: Math.min(parent.width, root.narrowMode ? 300 : 440)
                spacing: HydraTheme.space6
                visible: root.appState.sessionCount === 0

                Text {
                    id: emptyHeadline
                    text: root.narrowMode ? "> STANDBY" : "> COMMAND CHANNEL IDLE"
                    color: HydraTheme.textOnDark
                    font.family: HydraTheme.displayFamily
                    font.pixelSize: root.compactMode ? 24 : 30
                    font.bold: true
                    font.letterSpacing: 1.2
                }

                Row {
                    spacing: 0

                    Text {
                        text: "_"
                        color: HydraTheme.accentBronze
                        font.family: HydraTheme.monoFamily
                        font.pixelSize: emptyHeadline.font.pixelSize
                    }
                }

                Text {
                    width: parent.width
                    text: root.narrowMode
                          ? "open the rail, choose a target, then launch a shell"
                          : "choose a repository or worktree, then launch a detached shell to start the first live session"
                    wrapMode: Text.WordWrap
                    color: HydraTheme.textOnDarkMuted
                    font.family: HydraTheme.monoFamily
                    font.pixelSize: 10
                }

                Repeater {
                    model: root.narrowMode ? 2 : 3

                    StatusChip {
                        required property int index

                        width: parent.width
                        height: 18
                        toneColor: index === 2 ? HydraTheme.accentReady : HydraTheme.borderDark
                        textColor: index === 2 ? HydraTheme.accentReady : HydraTheme.textOnDarkMuted
                        fillOpacity: 0.48
                        borderOpacity: 0.6
                        text: index === 0
                              ? "[01] choose target"
                              : (index === 1 ? "[02] launch detached shell" : "[03] inspect live signal trace")
                    }
                }
            }

            ListView {
                id: sessionList

                anchors.fill: parent
                spacing: root.denseMode ? HydraTheme.space6 : HydraTheme.space8
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                interactive: contentHeight > height
                model: root.appState.sessionModel
                visible: root.appState.sessionCount > 0
                activeFocusOnTab: true
                keyNavigationEnabled: false

                Component.onCompleted: root.ensureSelection()

                Connections {
                    target: sessionList.model

                    function onModelAboutToBeReset() {
                        root.preservedSessionListContentY = sessionList.contentY
                    }

                    function onModelReset() {
                        if (root.preservedSessionListContentY < 0) {
                            root.ensureSelection()
                            return
                        }

                        const targetY = Math.min(root.preservedSessionListContentY,
                                                 Math.max(0, sessionList.contentHeight - sessionList.height))
                        Qt.callLater(function() {
                            sessionList.contentY = targetY
                            root.preservedSessionListContentY = -1
                            root.ensureSelection()
                        })
                    }
                }

                delegate: Item {
                    id: delegateRoot
                    required property string sessionId
                    required property string name
                    required property string repoName
                    required property string detailText
                    required property string statusDetail
                    required property string stateLabel
                    required property string stateTone
                    required property string safetyLabel
                    required property string safetyTone
                    required property string provenanceLabel
                    required property string provenanceTone
                    required property string activityLabel
                    required property bool approvalPending
                    required property string updatedAtText
                    required property var timelineEntries
                    required property bool canTerminate
                    required property bool selected
                    required property string alias
                    required property string categoryKey

                    readonly property bool channelActive: activityLabel === "ACTIVE"
                                                         || approvalPending || canTerminate
                    readonly property bool manualOrderingEnabled: !root.appState.sessionAutosortEnabled
                    readonly property real waveCenterY: root.refreshLeadY - (root.denseMode ? 14 : 18)
                    readonly property real localCenterY: y - sessionList.contentY + (height / 2)
                    readonly property real refreshDistance: Math.abs(localCenterY - waveCenterY)
                    readonly property real refreshLevel: root.refreshCascadeActive
                                                         ? Math.max(0.0,
                                                                    1.0 - (refreshDistance
                                                                           / Math.max(height * 1.2, 1)))
                                                         : 0.0
                    property real lastDragOffsetY: 0
                    readonly property bool dragActive: manualOrderingEnabled && reorderDrag.active

                    width: sessionList.width
                    height: sessionCard.implicitHeight
                    implicitHeight: sessionCard.implicitHeight
                    z: dragActive ? 20 : 0

                    Behavior on y {
                        NumberAnimation {
                            duration: root.denseMode ? 170 : 210
                            easing.type: Easing.InOutCubic
                        }
                    }

                    SessionCard {
                        id: sessionCard

                        anchors.left: parent.left
                        anchors.right: parent.right
                        sessionId: parent.sessionId
                        sessionName: parent.name
                        repoName: parent.repoName
                        detailText: parent.detailText
                        statusDetail: parent.statusDetail
                        stateLabel: parent.stateLabel
                        stateTone: parent.stateTone
                        showLaunchSafetyChip: root.showLaunchSafetyChips
                        launchSafetyLabel: parent.safetyLabel
                        launchSafetyTone: parent.safetyTone
                        provenanceLabel: parent.provenanceLabel
                        provenanceTone: parent.provenanceTone
                        activityLabel: parent.activityLabel
                        approvalPending: parent.approvalPending
                        updatedAtText: parent.updatedAtText
                        timelineEntries: parent.timelineEntries
                        canTerminate: parent.canTerminate
                        selected: parent.selected
                        alias: parent.alias
                        diagnosticsExpanded: root.expandedSessionId === parent.sessionId
                                             || (root.expandedSessionId.length === 0
                                                 && root.startupExpandedSessionName === parent.name)
                        channelActivityLevel: parent.channelActive ? 1.0 : 0.0
                        refreshWaveLevel: parent.refreshLevel
                        hoverHost: root.hoverHost
                        compactMode: root.compactMode
                        denseMode: root.denseMode
                        manualOrderingEnabled: parent.manualOrderingEnabled
                        dragActive: parent.dragActive
                        transform: Translate { y: delegateRoot.dragActive ? delegateRoot.lastDragOffsetY : 0 }
                        onTerminateRequested: targetSessionId => root.appState.terminateSession(targetSessionId)
                        onAliasChangeRequested: (targetSessionId, newAlias) => root.appState.setSessionAlias(targetSessionId, newAlias)
                        onAliasEditRequested: (targetSessionId, currentAlias) => root.aliasEditRequested(targetSessionId, currentAlias)
                        onSessionSelected: targetSessionId => {
                            root.appState.selectedSessionId = targetSessionId
                            root.sessionActivated(targetSessionId)
                        }
                        onDiagnosticsToggleRequested: targetSessionId => {
                            if (root.expandedSessionId === targetSessionId) {
                                root.expandedSessionId = ""
                                root.startupExpandedSessionName = ""
                                return
                            }
                            root.expandedSessionId = targetSessionId
                            root.startupExpandedSessionName = ""
                        }
                    }

                    DragHandler {
                        id: reorderDrag

                        enabled: parent.manualOrderingEnabled
                        target: null
                        xAxis.enabled: false
                        yAxis.enabled: true
                        onActiveTranslationChanged: parent.lastDragOffsetY = activeTranslation.y
                        onActiveChanged: {
                            if (active) {
                                root.appState.selectedSessionId = parent.sessionId
                                return
                            }

                            const releaseOffset = parent.lastDragOffsetY
                            parent.lastDragOffsetY = 0
                            if (Math.abs(releaseOffset) < HydraTheme.space8) {
                                return
                            }

                            const dropIndex = sessionList.indexAt(sessionList.width / 2,
                                                                  parent.y + (parent.height / 2) + releaseOffset)
                            const fallbackIndex = releaseOffset > 0 ? sessionList.count - 1 : 0
                            const targetIndex = dropIndex >= 0 ? dropIndex : fallbackIndex
                            if (root.appState.moveSessionToIndex(parent.sessionId, targetIndex)) {
                                sessionList.positionViewAtIndex(targetIndex, ListView.Contain)
                            }
                        }
                    }
                }
            }
        }
    }
}
