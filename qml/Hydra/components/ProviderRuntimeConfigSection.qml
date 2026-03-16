pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts 6.5
import Hydra.Backend 1.0
import "../styles"

ColumnLayout {
    id: root

    property AppState appState
    property color accentColor: "white"
    property string providerKey: ""
    property string safetyKey: "workspace-safe"
    property string modelId: ""
    property string apiKeyEnvVarName: ""
    property string apiKeyValue: ""
    property string hermesProfileMode: "global"
    property string hermesProfilePath: ""
    property bool supportsModelOverride: false
    property bool supportsApiKeyInjection: false
    property bool usesHermesProfiles: false
    property string apiKeyHelperText: ""
    property string apiKeyEnvVarPlaceholder: ""
    property string modelPlaceholderText: "Optional model id or alias"
    property string hermesProfilePathPlaceholder: ""
    property string hermesProfilePathHelperText: "Relative paths resolve from the selected repo root. Missing files fall back to ~/.hermes."
    readonly property bool backendReady: Boolean(root.appState)
    signal providerKeyEdited(string value)
    signal safetyKeyEdited(string value)
    signal modelIdEdited(string value)
    signal apiKeyEnvVarNameEdited(string value)
    signal apiKeyValueEdited(string value)
    signal hermesProfileModeEdited(string value)
    signal hermesProfilePathEdited(string value)

    spacing: HydraTheme.space8

    Text {
        text: "PROVIDER"
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
            model: root.backendReady ? root.appState.providerModel : null

            ActionChip {
                required property string providerKey
                required property string displayName
                required property bool available

                text: "[" + displayName + "]"
                active: providerKey === root.providerKey
                enabled: available
                opacity: available ? 1.0 : 0.5
                toneColor: active ? root.accentColor : HydraTheme.textOnDarkMuted
                textPixelSize: 10
                accessibleLabel: "Select " + displayName + " provider"
                onClicked: root.providerKeyEdited(providerKey)
            }
        }
    }

    Item { Layout.preferredHeight: HydraTheme.space4 }

    Text {
        text: "SAFETY MODE"
        color: HydraTheme.textOnDarkMuted
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        font.bold: true
        font.letterSpacing: 1.0
        Layout.leftMargin: HydraTheme.space16
        Layout.rightMargin: HydraTheme.space16
    }

    Row {
        Layout.leftMargin: HydraTheme.space16
        Layout.rightMargin: HydraTheme.space16
        spacing: HydraTheme.space6

        ActionChip {
            text: "[SANDBOXED]"
            active: root.safetyKey === "workspace-safe"
            toneColor: active ? HydraTheme.accentReady : root.accentColor
            textPixelSize: 10
            accessibleLabel: "Sandboxed safety mode"
            onClicked: root.safetyKeyEdited("workspace-safe")
        }

        ActionChip {
            text: "[BYPASS]"
            active: root.safetyKey === "bypass"
            toneColor: HydraTheme.danger
            textPixelSize: 10
            accessibleLabel: "Bypass safety mode"
            onClicked: root.safetyKeyEdited("bypass")
        }
    }

    Item { Layout.preferredHeight: HydraTheme.space4 }

    Text {
        visible: root.supportsModelOverride
        text: "MODEL ID"
        color: HydraTheme.textOnDarkMuted
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        font.bold: true
        font.letterSpacing: 1.0
        Layout.leftMargin: HydraTheme.space16
        Layout.rightMargin: HydraTheme.space16
    }

    Rectangle {
        visible: root.supportsModelOverride
        Layout.fillWidth: true
        Layout.leftMargin: HydraTheme.space16
        Layout.rightMargin: HydraTheme.space16
        implicitHeight: 42
        color: HydraTheme.withAlpha(root.accentColor, 0.06)
        border.width: 1
        border.color: modelField.activeFocus
                      ? HydraTheme.withAlpha(root.accentColor, 0.6)
                      : HydraTheme.withAlpha(root.accentColor, 0.24)
        radius: HydraTheme.radius6

        TextField {
            id: modelField
            anchors.fill: parent
            anchors.margins: HydraTheme.space6
            text: root.modelId
            onTextChanged: root.modelIdEdited(text)
            placeholderText: root.modelPlaceholderText
            font.family: HydraTheme.monoFamily
            font.pixelSize: 11
            color: HydraTheme.textOnDark
            placeholderTextColor: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.4)
            background: Item {}
        }
    }

    Item { visible: root.supportsModelOverride; Layout.preferredHeight: HydraTheme.space4 }

    Text {
        visible: root.supportsApiKeyInjection
        text: "API KEY OVERRIDE"
        color: HydraTheme.textOnDarkMuted
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        font.bold: true
        font.letterSpacing: 1.0
        Layout.leftMargin: HydraTheme.space16
        Layout.rightMargin: HydraTheme.space16
    }

    Text {
        visible: root.supportsApiKeyInjection
        text: root.apiKeyHelperText
        color: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.8)
        font.family: HydraTheme.bodyFamily
        font.pixelSize: 10
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
        Layout.leftMargin: HydraTheme.space16
        Layout.rightMargin: HydraTheme.space16
    }

    GridLayout {
        visible: root.supportsApiKeyInjection
        Layout.fillWidth: true
        Layout.leftMargin: HydraTheme.space16
        Layout.rightMargin: HydraTheme.space16
        columns: 2
        columnSpacing: HydraTheme.space8
        rowSpacing: HydraTheme.space6

        Text {
            text: "ENV"
            color: HydraTheme.textOnDarkMuted
            font.family: HydraTheme.monoFamily
            font.pixelSize: 10
            Layout.alignment: Qt.AlignVCenter
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 38
            color: HydraTheme.withAlpha(root.accentColor, 0.05)
            border.width: 1
            border.color: envVarField.activeFocus
                          ? HydraTheme.withAlpha(root.accentColor, 0.6)
                          : HydraTheme.withAlpha(root.accentColor, 0.22)
            radius: HydraTheme.radius6

            TextField {
                id: envVarField
                anchors.fill: parent
                anchors.margins: HydraTheme.space6
                text: root.apiKeyEnvVarName
                onTextChanged: root.apiKeyEnvVarNameEdited(text)
                placeholderText: root.apiKeyEnvVarPlaceholder
                font.family: HydraTheme.monoFamily
                font.pixelSize: 11
                color: HydraTheme.textOnDark
                background: Item {}
            }
        }

        Text {
            text: "KEY"
            color: HydraTheme.textOnDarkMuted
            font.family: HydraTheme.monoFamily
            font.pixelSize: 10
            Layout.alignment: Qt.AlignVCenter
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 38
            color: HydraTheme.withAlpha(root.accentColor, 0.05)
            border.width: 1
            border.color: apiKeyField.activeFocus
                          ? HydraTheme.withAlpha(root.accentColor, 0.6)
                          : HydraTheme.withAlpha(root.accentColor, 0.22)
            radius: HydraTheme.radius6

            TextField {
                id: apiKeyField
                anchors.fill: parent
                anchors.margins: HydraTheme.space6
                text: root.apiKeyValue
                onTextChanged: root.apiKeyValueEdited(text)
                echoMode: TextInput.Password
                placeholderText: "Leave blank to use existing provider auth"
                font.family: HydraTheme.monoFamily
                font.pixelSize: 11
                color: HydraTheme.textOnDark
                background: Item {}
            }
        }
    }

    Item { visible: root.supportsApiKeyInjection; Layout.preferredHeight: HydraTheme.space4 }

    Text {
        visible: root.usesHermesProfiles
        text: "HERMES PROFILE"
        color: HydraTheme.textOnDarkMuted
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        font.bold: true
        font.letterSpacing: 1.0
        Layout.leftMargin: HydraTheme.space16
        Layout.rightMargin: HydraTheme.space16
    }

    Flow {
        visible: root.usesHermesProfiles
        Layout.fillWidth: true
        Layout.leftMargin: HydraTheme.space16
        Layout.rightMargin: HydraTheme.space16
        spacing: HydraTheme.space6

        Repeater {
            model: root.backendReady ? root.appState.hermesProfileModeOptions() : []

            ActionChip {
                required property var modelData

                text: "[" + modelData.label + "]"
                active: root.hermesProfileMode === modelData.key
                toneColor: active ? root.accentColor : HydraTheme.textOnDarkMuted
                textPixelSize: 10
                accessibleLabel: "Select Hermes profile mode " + modelData.label
                onClicked: root.hermesProfileModeEdited(modelData.key)
            }
        }
    }

    Text {
        visible: root.usesHermesProfiles
        text: root.backendReady ? root.appState.hermesProfileModeSummary(root.hermesProfileMode) : ""
        color: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.8)
        font.family: HydraTheme.bodyFamily
        font.pixelSize: 10
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
        Layout.leftMargin: HydraTheme.space16
        Layout.rightMargin: HydraTheme.space16
    }

    Text {
        visible: root.usesHermesProfiles && root.hermesProfileMode === "repo-template"
        text: "TEMPLATE PATH"
        color: HydraTheme.textOnDarkMuted
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        font.bold: true
        font.letterSpacing: 1.0
        Layout.leftMargin: HydraTheme.space16
        Layout.rightMargin: HydraTheme.space16
    }

    Rectangle {
        visible: root.usesHermesProfiles && root.hermesProfileMode === "repo-template"
        Layout.fillWidth: true
        Layout.leftMargin: HydraTheme.space16
        Layout.rightMargin: HydraTheme.space16
        implicitHeight: 42
        color: HydraTheme.withAlpha(root.accentColor, 0.05)
        border.width: 1
        border.color: hermesProfilePathField.activeFocus
                      ? HydraTheme.withAlpha(root.accentColor, 0.6)
                      : HydraTheme.withAlpha(root.accentColor, 0.22)
        radius: HydraTheme.radius6

        TextField {
            id: hermesProfilePathField
            anchors.fill: parent
            anchors.margins: HydraTheme.space6
            text: root.hermesProfilePath
            onTextChanged: root.hermesProfilePathEdited(text)
            placeholderText: root.hermesProfilePathPlaceholder
            font.family: HydraTheme.monoFamily
            font.pixelSize: 11
            color: HydraTheme.textOnDark
            background: Item {}
        }
    }

    Text {
        visible: root.usesHermesProfiles && root.hermesProfileMode === "repo-template"
        text: root.hermesProfilePathHelperText
        color: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.8)
        font.family: HydraTheme.bodyFamily
        font.pixelSize: 10
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
        Layout.leftMargin: HydraTheme.space16
        Layout.rightMargin: HydraTheme.space16
    }

    Item { visible: root.usesHermesProfiles; Layout.preferredHeight: HydraTheme.space4 }
}
