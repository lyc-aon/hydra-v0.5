pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts 6.5
import Hydra.Backend 1.0
import "../styles"

Dialog {
    id: dialog

    required property AppState appState
    required property RouterState routerState

    property string localProviderKey: ""
    property string localSafetyKey: ""
    property string localModelId: ""
    property string localApiKeyEnvVarName: ""
    property string localApiKeyValue: ""
    property string localPresetKey: "standard"
    property string localPresetName: ""
    property string localUserDefaultContext: ""
    property string localSysprompt: ""
    property string localHermesProfileMode: "global"
    property string localHermesProfilePath: ""
    property string _lastProviderKey: ""
    readonly property bool supportsModelOverride: dialog.appState.providerSupportsModelOverride(dialog.localProviderKey)
    readonly property bool supportsApiKeyInjection: dialog.appState.defaultProviderApiKeyEnvVarName(dialog.localProviderKey).length > 0
    readonly property bool usesHermesProfiles: dialog.appState.providerUsesHermesProfiles(dialog.localProviderKey)
    readonly property bool presetIsBuiltIn: dialog.routerState.presetIsBuiltIn(dialog.localPresetKey)

    function presetLabelForKey(key) {
        const presets = dialog.routerState.presetOptions
        for (let index = 0; index < presets.length; ++index) {
            if (presets[index].key === key) {
                return presets[index].label
            }
        }
        return "CUSTOM PRESET"
    }

    function syncPresetEditor() {
        dialog.localSysprompt = dialog.routerState.presetPromptPreview(dialog.localPresetKey,
                                                                       dialog.localUserDefaultContext)
        dialog.localPresetName = dialog.presetLabelForKey(dialog.localPresetKey)
    }

    parent: Overlay.overlay
    modal: true
    dim: true
    focus: true
    x: Math.round((parent.width - width) * 0.5)
    y: Math.round((parent.height - height) * 0.5)
    width: Math.min(760, parent.width - 24)
    height: Math.min(920, parent.height - 24)
    padding: 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    onOpened: {
        const rpk = dialog.routerState.providerKey
        dialog.localProviderKey = rpk.length > 0
                                  ? rpk
                                  : dialog.appState.selectedProviderKey
        const rsk = dialog.routerState.launchSafetyKey
        dialog.localSafetyKey = rsk.length > 0 ? rsk : dialog.appState.selectedLaunchSafetyKey
        dialog.localModelId = dialog.routerState.modelId
        const envVarName = dialog.routerState.apiKeyEnvVarName
        dialog.localApiKeyEnvVarName = envVarName.length > 0
                                       ? envVarName
                                       : dialog.appState.defaultProviderApiKeyEnvVarName(dialog.localProviderKey)
        dialog.localApiKeyValue = dialog.routerState.apiKeyValue
        dialog.localPresetKey = dialog.routerState.presetKey.length > 0
                                ? dialog.routerState.presetKey
                                : "standard"
        dialog.localUserDefaultContext = dialog.routerState.userDefaultContext
        dialog.localHermesProfileMode = dialog.routerState.hermesProfileMode
        dialog.localHermesProfilePath = dialog.routerState.hermesProfilePath
        dialog._lastProviderKey = dialog.localProviderKey
        dialog.syncPresetEditor()
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

    onLocalPresetKeyChanged: dialog.syncPresetEditor()

    onLocalUserDefaultContextChanged: {
        if (dialog.localPresetKey === "strategic") {
            dialog.localSysprompt = dialog.routerState.presetPromptPreview(dialog.localPresetKey,
                                                                           dialog.localUserDefaultContext)
        }
    }

    Timer {
        id: syspromptFocusTimer
        interval: 60
        onTriggered: promptEditor.focusEditor()
    }

    background: Rectangle {
        color: HydraTheme.boardPanelMuted
        border.width: 1
        border.color: HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.5)
        radius: HydraTheme.radius10

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 2
            color: HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.6)
            radius: HydraTheme.radius10
        }
    }

    Overlay.modal: Rectangle {
        color: HydraTheme.withAlpha("#000000", 0.55)
    }

    contentItem: ColumnLayout {
        spacing: 0

        ScrollView {
            id: bodyScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AsNeeded
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            contentWidth: Math.max(0, availableWidth - 2)

            ColumnLayout {
                id: bodyColumn
                width: Math.max(0, bodyScroll.availableWidth - 2)
            spacing: HydraTheme.space8

            Item { Layout.preferredHeight: HydraTheme.space12 }

            Text {
                text: "ROUTER CONFIGURATION"
                color: HydraTheme.accentSteelBright
                font.family: HydraTheme.displayFamily
                font.pixelSize: 11
                font.bold: true
                font.letterSpacing: 1.2
                Layout.leftMargin: HydraTheme.space16
                Layout.rightMargin: HydraTheme.space16
            }

            Text {
                text: "Configure the AI orchestrator that routes prompts to worker sessions. Built-in router presets stay immutable; save your own variants alongside them so you can always switch back."
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
                accentColor: HydraTheme.accentSteelBright
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
                apiKeyHelperText: "Optional. Passed only to the launched router session and not persisted to QSettings."
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

            Text {
                text: "ROUTER PRESET"
                color: HydraTheme.textOnDarkMuted
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 1.0
                Layout.leftMargin: HydraTheme.space16
                Layout.rightMargin: HydraTheme.space16
            }

            Flow {
                Layout.fillWidth: true
                Layout.leftMargin: HydraTheme.space16
                Layout.rightMargin: HydraTheme.space16
                spacing: HydraTheme.space6

                Repeater {
                    model: dialog.routerState.presetOptions

                    ActionChip {
                        required property var modelData

                        text: "[" + modelData.label + "]"
                        active: dialog.localPresetKey === modelData.key
                        toneColor: active ? HydraTheme.accentSteelBright : HydraTheme.textOnDarkMuted
                        textPixelSize: 10
                        accessibleLabel: "Select router preset " + modelData.label
                        onClicked: dialog.localPresetKey = modelData.key
                    }
                }
            }

            Text {
                text: dialog.routerState.presetDescription(dialog.localPresetKey)
                color: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.85)
                font.family: HydraTheme.bodyFamily
                font.pixelSize: 10
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                Layout.leftMargin: HydraTheme.space16
                Layout.rightMargin: HydraTheme.space16
            }

            Text {
                text: dialog.presetIsBuiltIn
                      ? "Built-in presets are locked. Save a copy if you want to customize one."
                      : "Custom presets are editable and persist alongside the built-ins."
                color: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.72)
                font.family: HydraTheme.bodyFamily
                font.pixelSize: 10
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                Layout.leftMargin: HydraTheme.space16
                Layout.rightMargin: HydraTheme.space16
            }

            Text {
                text: "PRESET NAME"
                color: HydraTheme.textOnDarkMuted
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 1.0
                Layout.leftMargin: HydraTheme.space16
                Layout.rightMargin: HydraTheme.space16
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: HydraTheme.space16
                Layout.rightMargin: HydraTheme.space16
                implicitHeight: 42
                color: HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.05)
                border.width: 1
                border.color: presetNameField.activeFocus
                              ? HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.6)
                              : HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.22)
                radius: HydraTheme.radius6

                TextField {
                    id: presetNameField
                    anchors.fill: parent
                    anchors.margins: HydraTheme.space6
                    text: dialog.localPresetName
                    onTextChanged: dialog.localPresetName = text
                    placeholderText: dialog.presetIsBuiltIn ? "Name for a saved copy" : "Preset display name"
                    font.family: HydraTheme.monoFamily
                    font.pixelSize: 11
                    color: HydraTheme.textOnDark
                    background: Item {}
                }
            }

            Flow {
                Layout.fillWidth: true
                Layout.leftMargin: HydraTheme.space16
                Layout.rightMargin: HydraTheme.space16
                spacing: HydraTheme.space8
                Layout.alignment: Qt.AlignLeft

                ActionChip {
                    text: "[SAVE AS PRESET]"
                    toneColor: HydraTheme.accentSteelBright
                    textPixelSize: 10
                    accessibleLabel: "Save the current router prompt as a custom preset"
                    onClicked: {
                        const savedKey = dialog.routerState.saveCustomPreset("",
                                                                             dialog.localPresetName,
                                                                             dialog.localSysprompt)
                        if (savedKey.length > 0) {
                            dialog.localPresetKey = savedKey
                            dialog.localPresetName = dialog.presetLabelForKey(savedKey)
                        }
                    }
                }

                ActionChip {
                    visible: !dialog.presetIsBuiltIn
                    text: "[SAVE CHANGES]"
                    toneColor: HydraTheme.accentReady
                    textPixelSize: 10
                    accessibleLabel: "Save changes to the current custom router preset"
                    onClicked: {
                        const savedKey = dialog.routerState.saveCustomPreset(dialog.localPresetKey,
                                                                             dialog.localPresetName,
                                                                             dialog.localSysprompt)
                        if (savedKey.length > 0) {
                            dialog.localPresetKey = savedKey
                            dialog.localPresetName = dialog.presetLabelForKey(savedKey)
                        }
                    }
                }

                ActionChip {
                    visible: !dialog.presetIsBuiltIn
                    text: "[DELETE]"
                    toneColor: HydraTheme.danger
                    textPixelSize: 10
                    accessibleLabel: "Delete the current custom router preset"
                    onClicked: {
                        if (dialog.routerState.deleteCustomPreset(dialog.localPresetKey)) {
                            dialog.localPresetKey = "standard"
                        }
                    }
                }
            }

            Item { Layout.preferredHeight: HydraTheme.space4 }

            Text {
                text: "STRATEGIC DEFAULT CONTEXT"
                color: HydraTheme.textOnDarkMuted
                font.family: HydraTheme.monoFamily
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 1.0
                Layout.leftMargin: HydraTheme.space16
                Layout.rightMargin: HydraTheme.space16
            }

            Text {
                text: dialog.localPresetKey === "strategic"
                      ? "Used by the strategic router preset to enrich vague requests before routing."
                      : "Saved here so the strategic preset is ready when you switch back to it. Custom presets can also reference this context manually."
                color: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.8)
                font.family: HydraTheme.bodyFamily
                font.pixelSize: 10
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                Layout.leftMargin: HydraTheme.space16
                Layout.rightMargin: HydraTheme.space16
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: HydraTheme.space16
                Layout.rightMargin: HydraTheme.space16
                implicitHeight: 110
                color: HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.06)
                border.width: 1
                border.color: defaultContextArea.activeFocus
                              ? HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.6)
                              : HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.24)
                radius: HydraTheme.radius6
                clip: true

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: HydraTheme.space6

                    TextArea {
                        id: defaultContextArea
                        text: dialog.localUserDefaultContext
                        onTextChanged: dialog.localUserDefaultContext = text
                        font.family: HydraTheme.bodyFamily
                        font.pixelSize: 11
                        color: HydraTheme.textOnDark
                        selectionColor: HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.3)
                        selectedTextColor: HydraTheme.textOnDark
                        placeholderText: "Optional defaults such as preferred aesthetics, stack choices, coding style, platform constraints, or system information."
                        placeholderTextColor: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.4)
                        wrapMode: TextEdit.Wrap
                        background: Item {}
                    }
                }
            }

            Item { Layout.preferredHeight: HydraTheme.space4 }

            PromptEditorSection {
                id: promptEditor
                Layout.fillWidth: true
                accentColor: HydraTheme.accentSteelBright
                textValue: dialog.localSysprompt
                helperText: dialog.presetIsBuiltIn
                            ? "Preview only. Built-in presets cannot be edited in place."
                            : "Editable custom preset body."
                placeholderText: "Instructions for the router AI orchestrator..."
                preferredHeight: 220
                readOnly: dialog.presetIsBuiltIn
                onTextValueEdited: value => dialog.localSysprompt = value
                onEscapePressed: dialog.close()
            }

            Item { Layout.preferredHeight: HydraTheme.space12 }
        }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 1
            color: HydraTheme.withAlpha(HydraTheme.accentSteelBright, 0.18)
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: HydraTheme.space16
            Layout.rightMargin: HydraTheme.space16
            Layout.topMargin: HydraTheme.space10
            Layout.bottomMargin: HydraTheme.space12
            spacing: HydraTheme.space8

            Text {
                text: "Router changes apply here and relaunch the live router."
                color: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.78)
                font.family: HydraTheme.bodyFamily
                font.pixelSize: 10
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

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
                implicitWidth: 170
                implicitHeight: 26
                text: "[SAVE & RESTART ROUTER]"
                toneColor: HydraTheme.accentSteelBright
                textPixelSize: 10
                accessibleLabel: "Save configuration and relaunch router session"
                onClicked: {
                    let activePresetKey = dialog.localPresetKey
                    if (!dialog.presetIsBuiltIn) {
                        const savedKey = dialog.routerState.saveCustomPreset(dialog.localPresetKey,
                                                                             dialog.localPresetName,
                                                                             dialog.localSysprompt)
                        if (savedKey.length > 0) {
                            activePresetKey = savedKey
                            dialog.localPresetKey = savedKey
                        }
                    }
                    const providerKey = dialog.localProviderKey
                    const safetyKey = dialog.localSafetyKey
                    const userDefaultContext = dialog.localUserDefaultContext
                    const modelId = dialog.localModelId
                    const apiKeyEnvVarName = dialog.localApiKeyEnvVarName
                    const apiKeyValue = dialog.localApiKeyValue
                    const hermesProfileMode = dialog.localHermesProfileMode
                    const hermesProfilePath = dialog.localHermesProfilePath
                    HydraSounds.playCompletion()
                    dialog.close()
                    Qt.callLater(function() {
                        dialog.routerState.applyConfig(providerKey,
                                                       safetyKey,
                                                       activePresetKey,
                                                       userDefaultContext,
                                                       modelId,
                                                       apiKeyEnvVarName,
                                                       apiKeyValue,
                                                       hermesProfileMode,
                                                       hermesProfilePath)
                    })
                }
            }
        }
    }
}
