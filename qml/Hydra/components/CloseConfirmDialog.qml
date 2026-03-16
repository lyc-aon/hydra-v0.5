pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Controls 6.5
import "../styles"

Dialog {
    id: dialog

    required property var host

    function openDialog() {
        dialog.open()
    }

    parent: Overlay.overlay
    modal: true
    dim: true
    focus: true
    x: Math.round((host.width - width) * 0.5)
    y: Math.round((host.height - height) * 0.5)
    width: Math.min(472, host.width - (HydraTheme.space20 * 2))
    padding: HydraTheme.space16
    closePolicy: Popup.NoAutoClose

    background: Rectangle {
        radius: HydraTheme.radius10
        color: HydraTheme.railPanelStrong
        border.width: 1
        border.color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.72)
    }

    header: Item {
        implicitHeight: HydraTheme.space8
    }

    contentItem: Column {
        spacing: HydraTheme.space12

        Text {
            width: dialog.availableWidth
            text: dialog.host.appState.ownedLiveSessionCount === 1
                  ? "Close Hydra and shut down 1 active session?"
                  : "Close Hydra and shut down " + dialog.host.appState.ownedLiveSessionCount + " active sessions?"
            color: HydraTheme.textOnDark
            font.family: HydraTheme.displayFamily
            font.pixelSize: 22
            font.bold: true
            wrapMode: Text.WordWrap
        }

        Text {
            width: dialog.availableWidth
            text: "Hydra will end this window's live tmux sessions before exit. Resumable provider conversations will be stored in the Resume rail. This can take a few seconds."
            color: HydraTheme.textOnDark
            font.family: HydraTheme.bodyFamily
            font.pixelSize: 13
            wrapMode: Text.WordWrap
        }
    }

    Overlay.modal: Rectangle {
        color: HydraTheme.withAlpha(HydraTheme.shellDepth, 0.72)
    }

    footer: Row {
        width: dialog.availableWidth
        spacing: HydraTheme.space10
        layoutDirection: Qt.RightToLeft

        HydraButton {
            width: 156
            height: 36
            text: "CLOSE HYDRA"
            toneColor: HydraTheme.danger
            onTriggered: {
                dialog.host.closeAfterShutdownApproval = true
                dialog.close()
                Qt.callLater(function() {
                    dialog.host.close()
                })
            }
        }

        HydraButton {
            width: 118
            height: 36
            text: "CANCEL"
            toneColor: HydraTheme.accentSteelBright
            onTriggered: {
                dialog.host.closeAfterShutdownApproval = false
                dialog.close()
            }
        }
    }
}
