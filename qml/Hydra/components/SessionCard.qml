import QtQuick 6.5
import QtQuick.Layouts 6.5
import "../styles"

Rectangle {
    id: root

    property string sessionId: ""
    property string sessionName: ""
    property string repoName: ""
    property string detailText: ""
    property string statusDetail: ""
    property string stateLabel: ""
    property string stateTone: "thinking"
    property bool showLaunchSafetyChip: true
    property string launchSafetyLabel: ""
    property string launchSafetyTone: "ready"
    property string provenanceLabel: ""
    property string provenanceTone: "steel"
    property string activityLabel: "QUIET"
    property bool approvalPending: false
    property string updatedAtText: ""
    property var timelineEntries: []
    property bool canTerminate: false
    property bool selected: false
    property string alias: ""
    property bool diagnosticsExpanded: false
    property real channelActivityLevel: 0.0
    property real refreshWaveLevel: 0.0
    property var hoverHost: null
    property bool compactMode: false
    property bool denseMode: false
    property bool manualOrderingEnabled: false
    property bool dragActive: false

    property string _previousStateLabel: ""
    property bool _initialized: false

    signal terminateRequested(string sessionId)
    signal diagnosticsToggleRequested(string sessionId)
    signal sessionSelected(string sessionId)
    signal aliasChangeRequested(string sessionId, string newAlias)
    signal aliasEditRequested(string sessionId, string currentAlias)

    Component.onCompleted: {
        _previousStateLabel = stateLabel
        _initialized = true
    }

    onStateLabelChanged: {
        if (!_initialized)
            return

        if (stateLabel === "Awaiting Approval" && _previousStateLabel !== "Awaiting Approval") {
            HydraSounds.playApproval()
        }

        if (stateLabel === "Idle"
                && (_previousStateLabel === "Thinking" || _previousStateLabel === "Running Tool")) {
            HydraSounds.playCompletion()
        }

        _previousStateLabel = stateLabel
    }

    readonly property color stateColor: HydraTheme.sessionStateColor(root.stateTone)
    readonly property color activityColor: root.approvalPending
                                          ? HydraTheme.accentSignal
                                          : (root.activityLabel === "ACTIVE"
                                                 ? HydraTheme.accentReady
                                                 : (root.activityLabel === "INPUT"
                                                        ? HydraTheme.warning
                                                        : HydraTheme.accentSteelBright))
    readonly property color provenanceColor: {
        switch (root.provenanceTone) {
        case "ready":
            return HydraTheme.accentReady
        case "bronze":
            return HydraTheme.accentBronze
        case "phosphor":
            return HydraTheme.accentPhosphor
        case "danger":
            return HydraTheme.danger
        case "steel":
        default:
            return HydraTheme.accentSteelBright
        }
    }
    readonly property color safetyColor: {
        switch (root.launchSafetyTone) {
        case "danger":
            return HydraTheme.danger
        case "ready":
            return HydraTheme.accentReady
        case "bronze":
            return HydraTheme.accentBronze
        default:
            return HydraTheme.accentSteelBright
        }
    }
    readonly property bool pulsingState: root.approvalPending || root.canTerminate
                                         || stateLabel === "Starting"
                                         || stateLabel === "Thinking"
                                         || stateLabel === "Running Tool"
                                         || stateLabel === "Awaiting Approval"
                                         || stateLabel === "Waiting For Input"
    readonly property bool ogSteamTheme: HydraTheme.currentThemeId === "og_steam"
    readonly property bool evaTheme: HydraTheme.currentThemeId === "eva"
    readonly property int contentInset: root.denseMode ? HydraTheme.space8 : HydraTheme.space10
    readonly property int metaChipMinWidth: root.denseMode ? 46 : 52
    readonly property int railChipWidth: root.denseMode ? 64 : 72
    readonly property int railSpacing: root.denseMode ? HydraTheme.space4 : HydraTheme.space6
    readonly property int railUnitHeight: root.denseMode ? 20 : 22
    readonly property int railWidth: (root.railChipWidth * 2) + root.railSpacing
    readonly property int actionRailWidth: root.railWidth
    readonly property real refreshAuraLevel: Math.max(0.0, Math.min(1.0, root.refreshWaveLevel))
    readonly property real traceRevealTarget: root.diagnosticsExpanded && root.timelineEntries.length > 0 ? 1.0 : 0.0
    implicitHeight: contentColumn.implicitHeight + (root.contentInset * 2)
    radius: root.denseMode ? HydraTheme.radius4 : HydraTheme.radius6
    color: root.evaTheme
           ? (root.selected
              ? HydraTheme.withAlpha(HydraTheme.boardRowStrong, 0.96)
              : HydraTheme.withAlpha(HydraTheme.boardRow, 0.98))
           : (root.selected
              ? HydraTheme.withAlpha(HydraTheme.boardRowStrong, 0.98)
              : (root.canTerminate ? HydraTheme.boardRowStrong : HydraTheme.boardRow))
    border.width: 1
    border.color: root.dragActive
                  ? HydraTheme.withAlpha(HydraTheme.accentBronze, 0.74)
                  : (root.selected
                     ? HydraTheme.withAlpha(HydraTheme.panelRule, 0.82)
                     : HydraTheme.borderDark)
    clip: true
    scale: root.dragActive ? 1.01 : 1.0
    Accessible.role: Accessible.Button
    Accessible.name: root.sessionName.length > 0
                     ? "Select session " + root.sessionName
                     : "Select session"
    Accessible.onPressAction: root.sessionSelected(root.sessionId)

    TapHandler {
        acceptedButtons: Qt.LeftButton
        onTapped: root.sessionSelected(root.sessionId)
    }

    Behavior on scale {
        NumberAnimation {
            duration: HydraTheme.motionFast
            easing.type: Easing.OutCubic
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        visible: root.selected
        border.width: 1
        border.color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.34)
        radius: root.radius
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        opacity: root.evaTheme
                 ? Math.max(0.0, Math.min(0.36, root.channelActivityLevel * 0.4))
                 : Math.max(0.0, Math.min(1.0, root.channelActivityLevel))
        visible: opacity > 0.01
        border.width: 1
        border.color: HydraTheme.withAlpha(root.stateColor, root.evaTheme ? 0.12 : 0.24)
        gradient: Gradient {
            GradientStop { position: 0.0; color: HydraTheme.withAlpha(root.stateColor, root.evaTheme ? 0.01 : 0.05) }
            GradientStop { position: 0.18; color: HydraTheme.withAlpha(root.stateColor, root.evaTheme ? 0.025 : 0.12) }
            GradientStop { position: 0.5; color: HydraTheme.withAlpha(root.stateColor, root.evaTheme ? 0.045 : 0.18) }
            GradientStop { position: 0.82; color: HydraTheme.withAlpha(root.stateColor, root.evaTheme ? 0.025 : 0.11) }
            GradientStop { position: 1.0; color: HydraTheme.withAlpha(root.stateColor, root.evaTheme ? 0.01 : 0.04) }
        }

        Behavior on opacity {
            NumberAnimation {
                duration: HydraTheme.motionNormal
                easing.type: Easing.OutCubic
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        opacity: root.refreshAuraLevel * (root.evaTheme ? 0.24 : 0.84)
        visible: opacity > 0.01
        gradient: Gradient {
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 0.18; color: HydraTheme.withAlpha(HydraTheme.panelRule, root.evaTheme ? 0.02 : 0.05) }
            GradientStop { position: 0.42; color: HydraTheme.withAlpha(HydraTheme.panelRule, root.evaTheme ? 0.05 : 0.16) }
            GradientStop { position: 0.62; color: HydraTheme.withAlpha(HydraTheme.accentPhosphorSoft, root.evaTheme ? 0.03 : 0.1) }
            GradientStop { position: 0.82; color: HydraTheme.withAlpha(HydraTheme.panelRule, root.evaTheme ? 0.02 : 0.06) }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        opacity: root.refreshAuraLevel
        visible: opacity > 0.01
        border.width: 1
        border.color: HydraTheme.withAlpha(HydraTheme.accentPhosphorSoft, 0.18 + (root.refreshAuraLevel * 0.34))
        radius: root.radius
    }

    Rectangle {
        anchors.left: edgeBar.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: root.denseMode ? 12 : 16
        color: "transparent"
        opacity: root.refreshAuraLevel * 0.55
        visible: opacity > 0.01
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: HydraTheme.withAlpha(HydraTheme.accentPhosphorSoft, 0.22) }
            GradientStop { position: 0.58; color: HydraTheme.withAlpha(HydraTheme.panelRule, 0.08) }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }

    Rectangle {
        anchors.left: edgeBar.right
        anchors.top: parent.top
        anchors.right: parent.right
        height: 1
        color: HydraTheme.withAlpha(root.stateColor, 0.22)
    }

    Rectangle {
        id: edgeBar

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 3
        color: root.stateColor
        opacity: root.pulsingState ? 0.72 : 1.0
    }

    ColumnLayout {
        id: contentColumn

        anchors.fill: parent
        anchors.leftMargin: root.contentInset
        anchors.rightMargin: root.contentInset
        anchors.topMargin: root.contentInset
        anchors.bottomMargin: root.contentInset
        spacing: root.denseMode ? HydraTheme.space6 : HydraTheme.space8

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: root.denseMode ? HydraTheme.space8 : HydraTheme.space12

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                spacing: HydraTheme.space4

                RowLayout {
                    Layout.fillWidth: true
                    spacing: HydraTheme.space8

                    Text {
                        text: root.sessionName
                        color: HydraTheme.textOnLight
                        font.family: HydraTheme.displayFamily
                        font.pixelSize: root.denseMode ? 12 : 14
                        font.bold: true
                        font.letterSpacing: 0.7
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    Text {
                        visible: !root.compactMode && !root.denseMode
                        text: root.updatedAtText
                        color: HydraTheme.textOnLightSoft
                        font.pixelSize: 10
                        font.family: HydraTheme.monoFamily
                        horizontalAlignment: Text.AlignRight
                    }
                }

                Text {
                    visible: root.compactMode || root.denseMode
                    text: root.updatedAtText
                    color: HydraTheme.textOnLightSoft
                    font.pixelSize: 10
                    font.family: HydraTheme.monoFamily
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: HydraTheme.space8

                    Text {
                        text: root.repoName
                        color: HydraTheme.accentBronze
                        font.family: HydraTheme.monoFamily
                        font.pixelSize: 10
                        font.bold: true
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    ActionChip {
                        id: aliasChip

                        implicitWidth: Math.max(root.denseMode ? 52 : 58, aliasChipLabel.implicitWidth + (root.denseMode ? 12 : 16))
                        implicitHeight: root.denseMode ? 18 : 20
                        toneColor: HydraTheme.accentBronze
                        fillOpacity: 0.06
                        borderOpacity: 0.22
                        textPixelSize: root.denseMode ? 9 : 10
                        text: ""
                        accessibleLabel: root.alias.length > 0
                                         ? "Edit alias @" + root.alias.toLowerCase()
                                         : "Set session alias"
                        toolTipText: "Set a short alias for this session. Used by master terminal routing (@alias)."
                        hoverHost: root.hoverHost
                        onClicked: root.aliasEditRequested(root.sessionId, root.alias)

                        Text {
                            id: aliasChipLabel
                            anchors.centerIn: parent
                            text: root.alias.length > 0 ? "@" + root.alias.toLowerCase() : "[@alias]"
                            color: root.alias.length > 0 ? HydraTheme.accentBronze : HydraTheme.textOnDarkMuted
                            font.family: HydraTheme.monoFamily
                            font.pixelSize: root.denseMode ? 9 : 10
                            font.bold: true
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignTop | Qt.AlignRight
                Layout.minimumWidth: root.railWidth
                Layout.preferredWidth: root.railWidth
                Layout.maximumWidth: root.railWidth
                spacing: root.denseMode ? HydraTheme.space6 : HydraTheme.space8

                RowLayout {
                    Layout.minimumWidth: root.railWidth
                    Layout.preferredWidth: root.railWidth
                    Layout.maximumWidth: root.railWidth
                    spacing: root.railSpacing

                    ColumnLayout {
                        Layout.minimumWidth: root.railChipWidth
                        Layout.preferredWidth: root.railChipWidth
                        Layout.maximumWidth: root.railChipWidth
                        spacing: root.railSpacing

                        StatusChip {
                            toneColor: root.stateColor
                            textColor: root.stateColor
                            fillOpacity: 0.12
                            borderOpacity: 0.34
                            Layout.minimumWidth: root.railChipWidth
                            Layout.preferredWidth: root.railChipWidth
                            Layout.maximumWidth: root.railChipWidth
                            Layout.minimumHeight: root.railUnitHeight
                            Layout.preferredHeight: root.railUnitHeight
                            Layout.maximumHeight: root.railUnitHeight
                            minWidth: root.railChipWidth
                            horizontalPadding: root.denseMode ? HydraTheme.space6 : HydraTheme.space8
                            verticalPadding: HydraTheme.space2
                            textPixelSize: 10
                            text: root.stateLabel.toUpperCase()
                            toolTipText: "Hydra's current normalized session state."
                            hoverHost: root.hoverHost
                        }

                        StatusChip {
                            visible: root.showLaunchSafetyChip && root.launchSafetyLabel.length > 0
                            toneColor: root.safetyColor
                            textColor: root.safetyColor
                            fillOpacity: root.launchSafetyTone === "danger" ? 0.14 : 0.1
                            borderOpacity: root.launchSafetyTone === "danger" ? 0.42 : 0.28
                            Layout.minimumWidth: root.railChipWidth
                            Layout.preferredWidth: root.railChipWidth
                            Layout.maximumWidth: root.railChipWidth
                            Layout.minimumHeight: root.railUnitHeight
                            Layout.preferredHeight: root.railUnitHeight
                            Layout.maximumHeight: root.railUnitHeight
                            minWidth: root.railChipWidth
                            horizontalPadding: root.denseMode ? HydraTheme.space6 : HydraTheme.space8
                            verticalPadding: HydraTheme.space2
                            textPixelSize: 10
                            text: root.launchSafetyLabel.toUpperCase()
                            accessibleLabel: root.sessionName + " posture " + root.launchSafetyLabel
                            toolTipText: root.launchSafetyTone === "danger"
                                         ? "This session is running in bypass mode."
                                         : "This session is running in sandboxed mode."
                            hoverHost: root.hoverHost
                        }
                    }

                    ColumnLayout {
                        Layout.minimumWidth: root.railChipWidth
                        Layout.preferredWidth: root.railChipWidth
                        Layout.maximumWidth: root.railChipWidth
                        spacing: root.railSpacing

                        StatusChip {
                            visible: root.approvalPending
                            toneColor: HydraTheme.accentSignal
                            textColor: HydraTheme.accentSignal
                            fillOpacity: 0.12
                            borderOpacity: 0.3
                            Layout.minimumWidth: root.railChipWidth
                            Layout.preferredWidth: root.railChipWidth
                            Layout.maximumWidth: root.railChipWidth
                            Layout.minimumHeight: root.railUnitHeight
                            Layout.preferredHeight: root.railUnitHeight
                            Layout.maximumHeight: root.railUnitHeight
                            minWidth: root.metaChipMinWidth
                            horizontalPadding: root.denseMode ? HydraTheme.space6 : HydraTheme.space8
                            verticalPadding: HydraTheme.space2
                            textPixelSize: 10
                            text: "APPROVAL"
                            toolTipText: "Provider output indicates that human approval is currently required."
                            hoverHost: root.hoverHost
                        }

                        StatusChip {
                            visible: !root.approvalPending
                            toneColor: root.activityColor
                            textColor: root.activityColor
                            fillOpacity: 0.08
                            borderOpacity: 0.24
                            Layout.minimumWidth: root.railChipWidth
                            Layout.preferredWidth: root.railChipWidth
                            Layout.maximumWidth: root.railChipWidth
                            Layout.minimumHeight: root.railUnitHeight
                            Layout.preferredHeight: root.railUnitHeight
                            Layout.maximumHeight: root.railUnitHeight
                            minWidth: root.metaChipMinWidth
                            horizontalPadding: root.denseMode ? HydraTheme.space6 : HydraTheme.space8
                            verticalPadding: HydraTheme.space2
                            textPixelSize: 10
                            text: root.activityLabel
                            toolTipText: "Recent activity signal inferred from tmux output and persisted trace history."
                            hoverHost: root.hoverHost
                        }

                        StatusChip {
                            toneColor: root.provenanceColor
                            textColor: root.provenanceColor
                            fillOpacity: 0.08
                            borderOpacity: 0.24
                            Layout.minimumWidth: root.railChipWidth
                            Layout.preferredWidth: root.railChipWidth
                            Layout.maximumWidth: root.railChipWidth
                            Layout.minimumHeight: root.railUnitHeight
                            Layout.preferredHeight: root.railUnitHeight
                            Layout.maximumHeight: root.railUnitHeight
                            minWidth: root.metaChipMinWidth
                            horizontalPadding: root.denseMode ? HydraTheme.space6 : HydraTheme.space8
                            verticalPadding: HydraTheme.space2
                            textPixelSize: 10
                            text: root.provenanceLabel.toUpperCase()
                            toolTipText: "Most recent signal source Hydra trusted for this state."
                            hoverHost: root.hoverHost
                        }
                    }
                }

                Item {
                    Layout.preferredWidth: root.railWidth
                    Layout.minimumWidth: root.railWidth
                    Layout.maximumWidth: root.railWidth
                    implicitHeight: actionRail.implicitHeight

                    RowLayout {
                        id: actionRail

                        anchors.right: parent.right
                        spacing: root.railSpacing
                        width: root.actionRailWidth

                        ActionChip {
                            implicitWidth: root.railChipWidth
                            implicitHeight: root.railUnitHeight
                            text: root.diagnosticsExpanded ? "[HIDE]" : "[TRACE]"
                            toneColor: HydraTheme.accentSteelBright
                            active: root.diagnosticsExpanded
                            textPixelSize: 10
                            accessibleLabel: root.diagnosticsExpanded
                                             ? "Hide trace for " + root.sessionName
                                             : "Show trace for " + root.sessionName
                            toolTipText: "Reveal the persisted recent signal trace for this session."
                            hoverHost: root.hoverHost
                            onClicked: root.diagnosticsToggleRequested(root.sessionId)
                        }

                        ActionChip {
                            visible: root.canTerminate
                            implicitWidth: root.railChipWidth
                            implicitHeight: root.railUnitHeight
                            text: "[END]"
                            toneColor: HydraTheme.danger
                            textPixelSize: 10
                            accessibleLabel: root.sessionName.length > 0
                                             ? "End session " + root.sessionName
                                             : "End session"
                            toolTipText: "Kill this tmux session and mark it exited in Hydra."
                            hoverHost: root.hoverHost
                            onClicked: root.terminateRequested(root.sessionId)
                        }
                    }
                }
            }
        }

        Text {
            text: root.statusDetail
            color: HydraTheme.textOnLightMuted
            font.family: HydraTheme.bodyFamily
            font.pixelSize: root.denseMode ? 10 : 11
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Text {
            text: root.detailText.replace("  |  ", " // ")
            color: HydraTheme.textOnDarkMuted
            font.pixelSize: 10
            font.family: HydraTheme.monoFamily
            wrapMode: Text.WordWrap
            maximumLineCount: root.denseMode ? 2 : (root.compactMode ? 3 : 2)
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        Item {
            id: traceRevealContainer

            Layout.fillWidth: true
            visible: root.timelineEntries.length > 0 || reveal > 0.001
            implicitHeight: Math.round((timelinePanel.implicitHeight + (root.denseMode ? HydraTheme.space2 : HydraTheme.space4))
                                       * reveal)
            opacity: reveal
            clip: true

            property real reveal: root.traceRevealTarget

            Behavior on reveal {
                NumberAnimation {
                    duration: root.denseMode ? 170 : 210
                    easing.type: Easing.InOutCubic
                }
            }

            SessionTimelinePanel {
                id: timelinePanel

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                y: Math.round((1.0 - traceRevealContainer.reveal) * -(root.denseMode ? 8 : 10))
                opacity: traceRevealContainer.reveal
                visible: traceRevealContainer.reveal > 0.001
                entries: root.timelineEntries
                denseMode: root.denseMode
                hoverHost: root.hoverHost
            }
        }
    }
}
