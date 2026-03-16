pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts 6.5
import Hydra.Backend 1.0
import "../styles"

SurfacePanel {
    id: root

    required property AppState appState
    property var helpHost: null
    property bool denseMode: false
    property bool tightMode: false
    property bool canLaunch: false
    readonly property int laneInset: denseMode ? HydraTheme.space4 : HydraTheme.space5
    readonly property bool homeLaunch: root.appState.launchInHomeDirectory
    readonly property bool supportsModelOverride: root.appState.providerSupportsModelOverride(root.appState.selectedProviderKey)
    readonly property bool usesHermesProfiles: root.appState.providerUsesHermesProfiles(root.appState.selectedProviderKey)
    readonly property bool launchArmed: canLaunch
                                       && appState.selectedProviderKey.length > 0
                                       && appState.selectedLaunchSafetyKey.length > 0
    contentMargin: denseMode ? HydraTheme.space8 : HydraTheme.space10
    contentSpacing: denseMode ? HydraTheme.space6 : HydraTheme.space8
    showHexGrid: true
    panelColor: HydraTheme.railPanelStrong
    panelBorderColor: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.22)

    function requestHelp(topicId, sourceItem) {
        if (helpHost && helpHost.openQuickHelp) {
            helpHost.openQuickHelp(topicId, sourceItem)
        }
    }

    function launchTitle() {
        if (root.tightMode) {
            return "LAUNCH"
        }
        return "LAUNCH " + root.appState.selectedProviderName.toUpperCase()
    }

    function launchSubtitle() {
        if (!root.appState.tmuxAvailable) {
            return "tmux unavailable"
        }
        return root.appState.selectedLaunchSafetyLabel + " via tmux // F7"
    }

    function focusPrimaryControl() {
        targetLaunchChip.forceActiveFocus()
    }

    Connections {
        target: root.appState

        function onLaunchConfigurationChanged() {
            if (launchButton.armed) {
                launchButton.playArmBurst()
            }
        }
    }

    SectionHeader {
        Layout.fillWidth: true
        title: "EXECUTE"

        InfoDotButton {
            topicId: "execute"
            briefText: "Select the provider and sandbox mode for the current target, then launch it in Hydra's detached tmux lane."
            accessibleLabel: "Explain execute"
            hoverHost: root.helpHost
            onHelpRequested: (topicId, source) => root.requestHelp(topicId, source)
        }

        StatusChip {
            toneColor: root.canLaunch ? HydraTheme.accentReady : HydraTheme.accentMuted
            textColor: root.canLaunch ? HydraTheme.accentReady : HydraTheme.textOnDarkMuted
            fillOpacity: 0.1
            borderOpacity: 0.3
            minWidth: root.canLaunch ? 72 : 82
            text: root.canLaunch ? "READY" : "BLOCKED"
        }
    }

    Text {
        text: root.homeLaunch
              ? "Home Directory"
              : (root.appState.selectedRepoId.length > 0
              ? (root.appState.selectedWorktreeBranch.length > 0
                 ? root.appState.selectedWorktreeBranch
                 : root.appState.selectedRepoName)
              : "Select a repository")
        color: (root.homeLaunch || root.appState.selectedRepoId.length > 0)
               ? HydraTheme.textOnDark
               : HydraTheme.textOnDarkMuted
        font.family: HydraTheme.displayFamily
        font.pixelSize: root.denseMode ? 14 : 16
        font.bold: true
        elide: Text.ElideRight
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
    }

    Text {
        visible: !root.tightMode
        text: root.homeLaunch
              ? "Launch unbound in " + root.appState.homeDirectoryPath
              : (root.appState.selectedRepoId.length > 0
              ? (root.appState.selectedWorktreePath.length > 0
                 ? root.appState.selectedWorktreePath
                 : root.appState.repositoryRootPath)
              : "Choose a repository below, or switch to HOME to launch the selected CLI unbound.")
        color: (root.homeLaunch || root.appState.selectedRepoId.length > 0)
               ? HydraTheme.textOnLightSoft
               : HydraTheme.textOnDarkMuted
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        elide: (root.homeLaunch || root.appState.selectedRepoId.length > 0)
               ? Text.ElideMiddle
               : Text.ElideRight
        wrapMode: (root.homeLaunch || root.appState.selectedRepoId.length > 0)
                  ? Text.NoWrap
                  : Text.WordWrap
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
    }

    Text {
        text: "LAUNCH TARGET"
        color: HydraTheme.accentBronze
        font.family: HydraTheme.displayFamily
        font.pixelSize: 11
        font.bold: true
        font.letterSpacing: 0.9
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
        Layout.topMargin: HydraTheme.space8
    }

    Flow {
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
        spacing: HydraTheme.space6

        HydraButton {
            id: targetLaunchChip
            text: "TARGET"
            selected: !root.homeLaunch
            hoverHost: root.helpHost
            toolTipText: "Launch inside the selected repository root or linked worktree."
            accessibleLabel: "Select launch target bound repository"
            textFamily: HydraTheme.displayFamily
            textPixelSize: 11
            textLetterSpacing: 0.7
            minWidth: 98
            onTriggered: root.appState.launchInHomeDirectory = false
        }

        HydraButton {
            id: homeLaunchChip
            text: "HOME"
            selected: root.homeLaunch
            hoverHost: root.helpHost
            toolTipText: "Launch in your home directory with no bound repository target."
            accessibleLabel: "Select launch target home directory"
            textFamily: HydraTheme.displayFamily
            textPixelSize: 11
            textLetterSpacing: 0.7
            minWidth: 98
            onTriggered: root.appState.launchInHomeDirectory = true
        }
    }

    Text {
        text: root.homeLaunch
              ? "HOME launches behave like opening the CLI normally from your desktop: no repo root or worktree binding."
              : "TARGET launches bind the next shell to the selected repository root or linked worktree."
        color: HydraTheme.textOnLightSoft
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
    }

    Text {
        text: "PROVIDER"
        color: HydraTheme.accentBronze
        font.family: HydraTheme.displayFamily
        font.pixelSize: 11
        font.bold: true
        font.letterSpacing: 0.9
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
        Layout.topMargin: HydraTheme.space8
    }

    LaunchDropdownField {
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
        selectedKey: root.appState.selectedProviderKey
        selectedLabel: root.appState.selectedProviderName
        selectionAvailable: root.appState.selectedProviderAvailable
        optionModel: root.appState.providerModel
        helpHost: root.helpHost
        onOptionSelected: providerKey => root.appState.selectedProviderKey = providerKey
    }

    Text {
        text: root.appState.selectedProviderAvailable
              ? "Selected provider CLI is ready on this machine."
              : root.appState.selectedProviderStatusText
        color: root.appState.selectedProviderAvailable ? HydraTheme.textOnLightSoft : HydraTheme.danger
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
    }

    Text {
        visible: root.supportsModelOverride
        text: "MODEL ID"
        color: HydraTheme.accentBronze
        font.family: HydraTheme.displayFamily
        font.pixelSize: 11
        font.bold: true
        font.letterSpacing: 0.9
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
        Layout.topMargin: HydraTheme.space8
    }

    Rectangle {
        visible: root.supportsModelOverride
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
        implicitHeight: 40
        color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.06)
        border.width: 1
        border.color: workerModelField.activeFocus
                      ? HydraTheme.withAlpha(HydraTheme.accentBronze, 0.6)
                      : HydraTheme.withAlpha(HydraTheme.accentBronze, 0.24)
        radius: HydraTheme.radius6

        TextField {
            id: workerModelField
            anchors.fill: parent
            anchors.margins: HydraTheme.space6
            text: root.appState.selectedModelId
            onTextChanged: root.appState.selectedModelId = text
            placeholderText: "Optional model id or alias"
            font.family: HydraTheme.monoFamily
            font.pixelSize: 11
            color: HydraTheme.textOnDark
            placeholderTextColor: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.4)
            background: Item {}
            Accessible.name: "Launch model id"
        }
    }

    Text {
        visible: root.supportsModelOverride
        text: root.appState.selectedModelId.length > 0
              ? "Hydra will pass this model id directly to the selected provider."
              : "Leave blank to use the provider's default model."
        color: HydraTheme.textOnLightSoft
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
    }

    Text {
        visible: root.usesHermesProfiles
        text: "HERMES PROFILE"
        color: HydraTheme.accentBronze
        font.family: HydraTheme.displayFamily
        font.pixelSize: 11
        font.bold: true
        font.letterSpacing: 0.9
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
        Layout.topMargin: HydraTheme.space8
    }

    Flow {
        visible: root.usesHermesProfiles
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
        spacing: HydraTheme.space6

        Repeater {
            model: root.appState.hermesProfileModeOptions()

            HydraButton {
                required property var modelData

                text: modelData.label
                selected: root.appState.selectedHermesProfileMode === modelData.key
                hoverHost: root.helpHost
                toolTipText: root.appState.hermesProfileModeSummary(modelData.key)
                accessibleLabel: "Select Hermes profile mode " + modelData.label
                textFamily: HydraTheme.displayFamily
                textPixelSize: 11
                textLetterSpacing: 0.7
                minWidth: modelData.key === "repo-template" ? 146 : 110
                onTriggered: root.appState.selectedHermesProfileMode = modelData.key
            }
        }
    }

    Text {
        visible: root.usesHermesProfiles
        text: root.appState.hermesProfileModeSummary(root.appState.selectedHermesProfileMode)
        color: HydraTheme.textOnLightSoft
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
    }

    Text {
        visible: root.usesHermesProfiles && root.appState.selectedHermesProfileMode === "repo-template"
        text: "TEMPLATE PATH"
        color: HydraTheme.accentBronze
        font.family: HydraTheme.displayFamily
        font.pixelSize: 11
        font.bold: true
        font.letterSpacing: 0.9
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
    }

    Rectangle {
        visible: root.usesHermesProfiles && root.appState.selectedHermesProfileMode === "repo-template"
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
        implicitHeight: 40
        color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.06)
        border.width: 1
        border.color: hermesProfilePathField.activeFocus
                      ? HydraTheme.withAlpha(HydraTheme.accentBronze, 0.6)
                      : HydraTheme.withAlpha(HydraTheme.accentBronze, 0.24)
        radius: HydraTheme.radius6

        TextField {
            id: hermesProfilePathField
            anchors.fill: parent
            anchors.margins: HydraTheme.space6
            text: root.appState.selectedHermesProfilePath
            onTextChanged: root.appState.selectedHermesProfilePath = text
            placeholderText: root.appState.defaultHermesProfileTemplatePath()
            font.family: HydraTheme.monoFamily
            font.pixelSize: 11
            color: HydraTheme.textOnDark
            placeholderTextColor: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.4)
            background: Item {}
            Accessible.name: "Hermes repo template path"
        }
    }

    Text {
        visible: root.usesHermesProfiles && root.appState.selectedHermesProfileMode === "repo-template"
        text: "Relative paths resolve from the selected repo root. Missing files fall back to ~/.hermes."
        color: HydraTheme.textOnLightSoft
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
    }

    Text {
        text: "SANDBOX"
        color: HydraTheme.accentBronze
        font.family: HydraTheme.displayFamily
        font.pixelSize: 11
        font.bold: true
        font.letterSpacing: 0.9
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
        Layout.topMargin: HydraTheme.space8
    }

    Flow {
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
        spacing: HydraTheme.space6

        Repeater {
            model: root.appState.launchSafetyOptions

            HydraButton {
                required property var modelData

                text: modelData.label
                selected: root.appState.selectedLaunchSafetyKey === modelData.key
                hoverHost: root.helpHost
                toolTipText: modelData.key === "bypass"
                             ? "Launch with the provider's bypass or permission-skip mode when it exists."
                             : "Launch with the provider's sandboxed or permission-gated mode."
                accessibleLabel: "Select sandbox mode " + modelData.label
                textFamily: HydraTheme.displayFamily
                textPixelSize: 11
                textLetterSpacing: 0.7
                minWidth: 98
                onTriggered: root.appState.selectedLaunchSafetyKey = modelData.key
            }
        }
    }

    Text {
        text: root.appState.selectedLaunchSafetySummary
        color: HydraTheme.textOnLightSoft
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
    }

    LaunchActionButton {
        id: launchButton

        canLaunch: root.canLaunch
        armed: root.launchArmed
        denseMode: root.denseMode
        hoverHost: root.helpHost
        titleText: root.launchTitle()
        subtitleText: root.launchSubtitle()
        activeHoverText: "Start " + root.appState.selectedProviderName + " in Hydra's detached tmux lane."
        inactiveHoverText: "Select a target, choose an available provider, and confirm tmux is available before launching."
        Layout.fillWidth: true
        Layout.leftMargin: root.laneInset
        Layout.rightMargin: root.laneInset
        onTriggered: root.appState.launchSelectedRepoSession()
    }
}
