pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts 6.5
import Hydra.Backend 1.0
import "../styles"

Dialog {
    id: dialog

    required property AppState appState
    required property MasterState masterState

    property string localProviderKey: ""
    property string localSafetyKey: ""
    property string localModelId: ""
    property string localApiKeyEnvVarName: ""
    property string localApiKeyValue: ""
    property string localSysprompt: ""
    property string localHermesProfileMode: "global"
    property string localHermesProfilePath: ""
    property string _lastProviderKey: ""
    readonly property bool supportsModelOverride: dialog.appState.providerSupportsModelOverride(dialog.localProviderKey)
    readonly property bool supportsApiKeyInjection: dialog.appState.defaultProviderApiKeyEnvVarName(dialog.localProviderKey).length > 0
    readonly property bool usesHermesProfiles: dialog.appState.providerUsesHermesProfiles(dialog.localProviderKey)

    parent: Overlay.overlay
    modal: true
    dim: true
    focus: true
    x: Math.round((parent.width - width) * 0.5)
    y: Math.round((parent.height - height) * 0.28)
    width: Math.min(520, parent.width - 40)
    padding: 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    onOpened: {
        const mpk = dialog.masterState.providerKey
        dialog.localProviderKey = mpk.length > 0 ? mpk : dialog.appState.selectedProviderKey
        const msk = dialog.masterState.launchSafetyKey
        dialog.localSafetyKey = msk.length > 0 ? msk : dialog.appState.selectedLaunchSafetyKey
        dialog.localModelId = dialog.masterState.modelId
        const envVarName = dialog.masterState.apiKeyEnvVarName
        dialog.localApiKeyEnvVarName = envVarName.length > 0
                                       ? envVarName
                                       : dialog.appState.defaultProviderApiKeyEnvVarName(dialog.localProviderKey)
        dialog.localApiKeyValue = dialog.masterState.apiKeyValue
        const storedPrompt = dialog.masterState.sysprompt
        dialog.localSysprompt = storedPrompt.length > 0 ? storedPrompt : dialog.masterState.defaultSysprompt()
        dialog.localHermesProfileMode = dialog.masterState.hermesProfileMode
        dialog.localHermesProfilePath = dialog.masterState.hermesProfilePath
        dialog._lastProviderKey = dialog.localProviderKey
        syspromptFocusTimer.restart()
    }

    onClosed: syspromptFocusTimer.stop()
    onLocalProviderKeyChanged: {
        const previousDefault = dialog.appState.defaultProviderApiKeyEnvVarName(dialog._lastProviderKey)
        if (dialog.localApiKeyEnvVarName.length === 0 || dialog.localApiKeyEnvVarName === previousDefault) {
            dialog.localApiKeyEnvVarName = dialog.appState.defaultProviderApiKeyEnvVarName(dialog.localProviderKey)
        }
        dialog._lastProviderKey = dialog.localProviderKey
    }

    Timer {
        id: syspromptFocusTimer
        interval: 60
        onTriggered: promptEditor.focusEditor()
    }

    background: Rectangle {
        color: HydraTheme.boardPanelMuted
        border.width: 1
        border.color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.5)
        radius: HydraTheme.radius10

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 2
            color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.6)
            radius: HydraTheme.radius10
        }
    }

    Overlay.modal: Rectangle {
        color: HydraTheme.withAlpha("#000000", 0.55)
    }

    contentItem: ColumnLayout {
        spacing: HydraTheme.space8

        Item { Layout.preferredHeight: HydraTheme.space12 }

        Text {
            text: "MASTER CONFIGURATION"
            color: HydraTheme.accentBronze
            font.family: HydraTheme.displayFamily
            font.pixelSize: 11
            font.bold: true
            font.letterSpacing: 1.2
            Layout.leftMargin: HydraTheme.space16
            Layout.rightMargin: HydraTheme.space16
        }

        Text {
            text: "Configure the provider, safety mode, and system prompt for the master terminal session."
            color: HydraTheme.textOnDarkMuted
            font.family: HydraTheme.bodyFamily
            font.pixelSize: 11
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            Layout.leftMargin: HydraTheme.space16
            Layout.rightMargin: HydraTheme.space16
        }

        ProviderRuntimeConfigSection {
            Layout.fillWidth: true
            appState: dialog.appState
            accentColor: HydraTheme.accentBronze
            providerKey: dialog.localProviderKey
            safetyKey: dialog.localSafetyKey
            modelId: dialog.localModelId
            apiKeyEnvVarName: dialog.localApiKeyEnvVarName
            apiKeyValue: dialog.localApiKeyValue
            hermesProfileMode: dialog.localHermesProfileMode
            hermesProfilePath: dialog.localHermesProfilePath
            supportsModelOverride: dialog.supportsModelOverride
            supportsApiKeyInjection: dialog.supportsApiKeyInjection
            usesHermesProfiles: dialog.usesHermesProfiles
            apiKeyHelperText: "Optional. Passed only to the launched master session and not persisted to QSettings."
            apiKeyEnvVarPlaceholder: dialog.appState.defaultProviderApiKeyEnvVarName(dialog.localProviderKey)
            hermesProfilePathPlaceholder: dialog.appState.defaultHermesProfileTemplatePath()
            onProviderKeyEdited: value => dialog.localProviderKey = value
            onSafetyKeyEdited: value => dialog.localSafetyKey = value
            onModelIdEdited: value => dialog.localModelId = value
            onApiKeyEnvVarNameEdited: value => dialog.localApiKeyEnvVarName = value
            onApiKeyValueEdited: value => dialog.localApiKeyValue = value
            onHermesProfileModeEdited: value => dialog.localHermesProfileMode = value
            onHermesProfilePathEdited: value => dialog.localHermesProfilePath = value
        }

        PromptEditorSection {
            id: promptEditor
            Layout.fillWidth: true
            accentColor: HydraTheme.accentBronze
            textValue: dialog.localSysprompt
            preferredHeight: 160
            placeholderText: "Instructions for the master AI..."
            onTextValueEdited: value => dialog.localSysprompt = value
            onEscapePressed: dialog.close()
        }

        Item { Layout.preferredHeight: HydraTheme.space4 }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: HydraTheme.space16
            Layout.rightMargin: HydraTheme.space16
            spacing: HydraTheme.space8

            Item { Layout.fillWidth: true }

            ActionChip {
                implicitWidth: 72
                implicitHeight: 26
                text: "[CANCEL]"
                toneColor: HydraTheme.accentSteelBright
                textPixelSize: 10
                accessibleLabel: "Cancel configuration"
                onClicked: dialog.close()
            }

            ActionChip {
                implicitWidth: 130
                implicitHeight: 26
                text: "[APPLY & RELAUNCH]"
                toneColor: HydraTheme.accentBronze
                textPixelSize: 10
                accessibleLabel: "Apply configuration and relaunch master session"
                onClicked: {
                    const providerKey = dialog.localProviderKey
                    const safetyKey = dialog.localSafetyKey
                    const sysprompt = dialog.localSysprompt
                    const modelId = dialog.localModelId
                    const apiKeyEnvVarName = dialog.localApiKeyEnvVarName
                    const apiKeyValue = dialog.localApiKeyValue
                    const hermesProfileMode = dialog.localHermesProfileMode
                    const hermesProfilePath = dialog.localHermesProfilePath
                    HydraSounds.playCompletion()
                    dialog.close()
                    Qt.callLater(function() {
                        dialog.masterState.applyConfig(providerKey,
                                                       safetyKey,
                                                       sysprompt,
                                                       modelId,
                                                       apiKeyEnvVarName,
                                                       apiKeyValue,
                                                       hermesProfileMode,
                                                       hermesProfilePath)
                    })
                }
            }
        }

        Item { Layout.preferredHeight: HydraTheme.space12 }
    }
}
