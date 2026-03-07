pragma Singleton
import QtQuick 6.5

QtObject {
    readonly property color shellBg: "#323A2C"
    readonly property color shellDepth: "#282A16"
    readonly property color shellDepthSoft: "#3D4636"
    readonly property color shellFrame: "#404838"
    readonly property color railBg: "#404838"
    readonly property color railPanel: "#4C5844"
    readonly property color railPanelStrong: "#505848"
    readonly property color railCard: "#4D5945"
    readonly property color railCardSelected: "#586050"
    readonly property color boardBg: "#404838"
    readonly property color boardPanel: "#4C5844"
    readonly property color boardPanelMuted: "#444C3C"
    readonly property color boardCard: "#505844"
    readonly property color boardRow: "#4C5440"
    readonly property color boardRowStrong: "#586050"
    readonly property color borderDark: "#707868"
    readonly property color borderLight: "#9EA28C"
    readonly property color borderFocus: "#D2D3C0"
    readonly property color gridLine: "#586050"
    readonly property color textOnDark: "#D2D3C0"
    readonly property color textOnDarkMuted: "#9EA28C"
    readonly property color textOnLight: "#D2D3C0"
    readonly property color textOnLightMuted: "#9EA28C"
    readonly property color textOnLightSoft: "#818A78"
    readonly property color accentBronze: "#939277"
    readonly property color accentEmber: "#916C47"
    readonly property color accentReady: "#D2D3C0"
    readonly property color accentReadyDeep: "#77816F"
    readonly property color accentOrange: "#707868"
    readonly property color accentOrangeStrong: "#9EA28C"
    readonly property color accentMuted: "#656E5A"
    readonly property color accentSteel: "#586050"
    readonly property color accentSteelBright: "#818A78"
    readonly property color accentCream: "#EBEFE8"
    readonly property color accentSignal: "#9B6A39"
    readonly property color accentSignalDeep: "#615C2C"
    readonly property color accentPhosphor: "#D2D3C0"
    readonly property color accentPhosphorSoft: "#9EA28C"
    readonly property color danger: "#9B6A39"
    readonly property color warning: "#939277"

    readonly property int space2: 2
    readonly property int space4: 4
    readonly property int space6: 6
    readonly property int space8: 8
    readonly property int space10: 10
    readonly property int space12: 12
    readonly property int space14: 14
    readonly property int space16: 16
    readonly property int space18: 18
    readonly property int space20: 20
    readonly property int space24: 24
    readonly property int space32: 32
    readonly property int shellMargin: 14

    readonly property int radius4: 2
    readonly property int radius6: 2
    readonly property int radius8: 3
    readonly property int radius10: 4
    readonly property int radius12: 5
    readonly property int radius14: 6
    readonly property int radius18: 8
    readonly property int radius22: 10

    readonly property int motionFast: 90
    readonly property int motionNormal: 140
    readonly property int motionSlow: 220
    readonly property int motionAmbient: 2800
    readonly property int motionDrift: 5200
    readonly property bool ambientEnabled: true

    readonly property string displayFamily: "Roboto Condensed"
    readonly property string bodyFamily: "Noto Sans"
    readonly property string monoFamily: "JetBrainsMono Nerd Font Mono"

    function withAlpha(colorValue, alphaValue) {
        return Qt.alpha(colorValue, alphaValue)
    }
}
