pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts 6.5
import "../styles"

ColumnLayout {
    id: root

    required property color accentColor
    required property string textValue
    property string title: "SYSTEM PROMPT"
    property string helperText: ""
    property string placeholderText: ""
    property bool readOnly: false
    property int preferredHeight: 180
    property alias editor: promptArea
    signal textValueEdited(string value)
    signal escapePressed()

    spacing: HydraTheme.space4

    function focusEditor() {
        promptArea.forceActiveFocus()
    }

    Text {
        text: root.title
        color: HydraTheme.textOnDarkMuted
        font.family: HydraTheme.monoFamily
        font.pixelSize: 10
        font.bold: true
        font.letterSpacing: 1.0
        Layout.leftMargin: HydraTheme.space16
        Layout.rightMargin: HydraTheme.space16
    }

    Text {
        visible: root.helperText.length > 0
        text: root.helperText
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
        implicitHeight: root.preferredHeight
        color: HydraTheme.withAlpha(root.accentColor, 0.06)
        border.width: 1
        border.color: promptArea.activeFocus
                      ? HydraTheme.withAlpha(root.accentColor, 0.6)
                      : HydraTheme.withAlpha(root.accentColor, 0.24)
        radius: HydraTheme.radius6
        clip: true

        Behavior on border.color {
            ColorAnimation { duration: HydraTheme.motionFast }
        }

        ScrollView {
            anchors.fill: parent
            anchors.margins: HydraTheme.space6

            TextArea {
                id: promptArea

                text: root.textValue
                onTextChanged: root.textValueEdited(text)
                readOnly: root.readOnly
                font.family: HydraTheme.monoFamily
                font.pixelSize: 11
                color: HydraTheme.textOnDark
                selectionColor: HydraTheme.withAlpha(root.accentColor, 0.3)
                selectedTextColor: HydraTheme.textOnDark
                placeholderText: root.placeholderText
                placeholderTextColor: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.4)
                wrapMode: TextEdit.Wrap
                background: Item {}
                Keys.onEscapePressed: root.escapePressed()
            }
        }
    }
}
