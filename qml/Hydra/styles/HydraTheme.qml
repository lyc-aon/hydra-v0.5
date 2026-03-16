pragma Singleton
import QtQuick 6.5

QtObject {
    id: root

    property string currentThemeId: "og_steam"

    readonly property var palettes: ({
        "og_steam": {
            shellBg: "#3D4435",
            shellDepth: "#31382A",
            shellDepthSoft: "#384032",
            shellFrame: "#3C4435",
            railBg: "#3C4435",
            railPanel: "#46503E",
            railPanelStrong: "#4A5542",
            railCard: "#495441",
            railCardSelected: "#586250",
            boardBg: "#3C4435",
            boardPanel: "#46503E",
            boardPanelMuted: "#3D4537",
            boardCard: "#495441",
            boardRow: "#4A5542",
            boardRowStrong: "#586250",
            borderDark: "#7C856F",
            borderLight: "#B7BDA8",
            borderFocus: "#E0E6D7",
            gridLine: "#707A65",
            textOnDark: "#D1DAC7",
            textOnDarkMuted: "#BFC7B5",
            textOnLight: "#D1DAC7",
            textOnLightMuted: "#B5BEAB",
            textOnLightSoft: "#B5BDA9",
            accentBronze: "#BDBA73",
            accentEmber: "#9C995C",
            accentReady: "#C8CEAB",
            accentReadyDeep: "#69735E",
            accentOrange: "#8D8E5A",
            accentOrangeStrong: "#D0CA76",
            accentMuted: "#919A88",
            accentSteel: "#75806B",
            accentSteelBright: "#BCC3AF",
            accentCream: "#D1DAC7",
            accentSignal: "#CCBF68",
            accentSignalDeep: "#615F31",
            accentPhosphor: "#D1DAC7",
            accentPhosphorSoft: "#BAC2AE",
            accentHermes: "#75806B",
            accentHermesBright: "#AFB8A5",
            ambientRail: "#505A49",
            ambientRailBright: "#939C84",
            ambientTrace: "#75806B",
            ambientTraceBright: "#BCC3AF",
            danger: "#D7B277",
            warning: "#CCBF68",
            repositoryAccent: "#69735E",
            panelLead: "#69735E",
            panelLeadSoft: "#BDBA73",
            panelRule: "#BAC2AE",
            themeControlFill: "#69735E",
            themeControlTone: "#C8CEAB",
            themeControlText: "#D1DAC7",
            railSignalPrimary: "#69735E",
            railSignalSecondary: "#BDBA73",
            railSignalTertiary: "#BCC3AF",
            railPacketColor: "#C8CEAB",
            railPulseColor: "#BAC2AE",
            railFieldTint: "#505A49"
        },
        "hermes_veil": {
            shellBg: "#000000",
            shellDepth: "#000000",
            shellDepthSoft: "#020812",
            shellFrame: "#061325",
            railBg: "#000000",
            railPanel: "#030A14",
            railPanelStrong: "#07101F",
            railCard: "#081224",
            railCardSelected: "#0E1C36",
            boardBg: "#000000",
            boardPanel: "#020913",
            boardPanelMuted: "#01060D",
            boardCard: "#081224",
            boardRow: "#07101F",
            boardRowStrong: "#0E1C36",
            borderDark: "#1555C0",
            borderLight: "#47A6FF",
            borderFocus: "#8BDEFF",
            gridLine: "#0D2442",
            textOnDark: "#E8F4FF",
            textOnDarkMuted: "#86A9D6",
            textOnLight: "#E8F4FF",
            textOnLightMuted: "#A8C8F0",
            textOnLightSoft: "#7293C5",
            accentBronze: "#1F8DFF",
            accentEmber: "#0D57C8",
            accentReady: "#66C7FF",
            accentReadyDeep: "#0E3D86",
            accentOrange: "#2A74E6",
            accentOrangeStrong: "#63B7FF",
            accentMuted: "#355784",
            accentSteel: "#0D3D75",
            accentSteelBright: "#7ACFFF",
            accentCream: "#E8F4FF",
            accentSignal: "#25A4FF",
            accentSignalDeep: "#0A3263",
            accentPhosphor: "#66C7FF",
            accentPhosphorSoft: "#9ADDFF",
            accentHermes: "#1F8DFF",
            accentHermesBright: "#8BDEFF",
            ambientRail: "#08172D",
            ambientRailBright: "#47A6FF",
            ambientTrace: "#0B203E",
            ambientTraceBright: "#8BDEFF",
            danger: "#FF6B5E",
            warning: "#F2BF52",
            repositoryAccent: "#1F8DFF",
            panelLead: "#8BDEFF",
            panelLeadSoft: "#1F8DFF",
            panelRule: "#47A6FF",
            themeControlFill: "#07101F",
            themeControlTone: "#63B7FF",
            themeControlText: "#E8F4FF",
            railSignalPrimary: "#8BDEFF",
            railSignalSecondary: "#1F8DFF",
            railSignalTertiary: "#47A6FF",
            railPacketColor: "#63B7FF",
            railPulseColor: "#8BDEFF",
            railFieldTint: "#08172D"
        },
        "openai": {
            shellBg: "#F7F7F5",
            shellDepth: "#EEF0EB",
            shellDepthSoft: "#E5E8E1",
            shellFrame: "#D0D5CC",
            railBg: "#F7F7F5",
            railPanel: "#EFF2EC",
            railPanelStrong: "#E7EAE3",
            railCard: "#F7F7F4",
            railCardSelected: "#E9ECE5",
            boardBg: "#F7F7F5",
            boardPanel: "#F0F2ED",
            boardPanelMuted: "#ECEFE8",
            boardCard: "#F7F7F4",
            boardRow: "#F3F5F0",
            boardRowStrong: "#E7EAE3",
            borderDark: "#D0D5CC",
            borderLight: "#FFFFFF",
            borderFocus: "#101113",
            gridLine: "#DEE2D8",
            textOnDark: "#101113",
            textOnDarkMuted: "#6A726A",
            textOnLight: "#101113",
            textOnLightMuted: "#6A726A",
            textOnLightSoft: "#949B92",
            accentBronze: "#101113",
            accentEmber: "#2B312D",
            accentReady: "#10A37F",
            accentReadyDeep: "#0A6A53",
            accentOrange: "#6A726A",
            accentOrangeStrong: "#101113",
            accentMuted: "#A2A9A0",
            accentSteel: "#68746D",
            accentSteelBright: "#9BA59D",
            accentCream: "#FCFCFA",
            accentSignal: "#10A37F",
            accentSignalDeep: "#0A6A53",
            accentPhosphor: "#10A37F",
            accentPhosphorSoft: "#70C6B2",
            accentHermes: "#2B312D",
            accentHermesBright: "#68746D",
            ambientRail: "#D8DDD3",
            ambientRailBright: "#10A37F",
            ambientTrace: "#CED3CA",
            ambientTraceBright: "#68746D",
            danger: "#BE624F",
            warning: "#A88A49",
            repositoryAccent: "#101113",
            panelLead: "#101113",
            panelLeadSoft: "#D8DDD3",
            panelRule: "#D0D5CC",
            themeControlFill: "#E7EAE3",
            themeControlTone: "#101113",
            themeControlText: "#101113",
            railSignalPrimary: "#101113",
            railSignalSecondary: "#6A726A",
            railSignalTertiary: "#10A37F",
            railPacketColor: "#101113",
            railPulseColor: "#10A37F",
            railFieldTint: "#D8DDD3"
        },
        "chatgpt": {
            shellBg: "#343541",
            shellDepth: "#202123",
            shellDepthSoft: "#444654",
            shellFrame: "#565869",
            railBg: "#202123",
            railPanel: "#2A2B32",
            railPanelStrong: "#343541",
            railCard: "#353740",
            railCardSelected: "#444654",
            boardBg: "#343541",
            boardPanel: "#40414F",
            boardPanelMuted: "#353742",
            boardCard: "#444654",
            boardRow: "#3A3B49",
            boardRowStrong: "#4A4C5C",
            borderDark: "#565869",
            borderLight: "#ACACBE",
            borderFocus: "#10A37F",
            gridLine: "#4A4C5C",
            textOnDark: "#ECECF1",
            textOnDarkMuted: "#ACACBE",
            textOnLight: "#ECECF1",
            textOnLightMuted: "#C7C7D1",
            textOnLightSoft: "#8E8EA0",
            accentBronze: "#10A37F",
            accentEmber: "#0B7158",
            accentReady: "#19C37D",
            accentReadyDeep: "#0A7158",
            accentOrange: "#8E8EA0",
            accentOrangeStrong: "#D9D9E3",
            accentMuted: "#6D7185",
            accentSteel: "#565869",
            accentSteelBright: "#ACACBE",
            accentCream: "#ECECF1",
            accentSignal: "#10A37F",
            accentSignalDeep: "#075746",
            accentPhosphor: "#19C37D",
            accentPhosphorSoft: "#6EE7B7",
            accentHermes: "#8E8EA0",
            accentHermesBright: "#D9D9E3",
            ambientRail: "#2B2D36",
            ambientRailBright: "#10A37F",
            ambientTrace: "#40414F",
            ambientTraceBright: "#C7C7D1",
            danger: "#F87171",
            warning: "#FBBF24",
            repositoryAccent: "#10A37F",
            panelLead: "#D9D9E3",
            panelLeadSoft: "#565869",
            panelRule: "#8E8EA0",
            themeControlFill: "#2A2B32",
            themeControlTone: "#D9D9E3",
            themeControlText: "#ECECF1",
            railSignalPrimary: "#10A37F",
            railSignalSecondary: "#8E8EA0",
            railSignalTertiary: "#D9D9E3",
            railPacketColor: "#10A37F",
            railPulseColor: "#6EE7B7",
            railFieldTint: "#2B2D36"
        },
        "claude_paper": {
            shellBg: "#FAF9F5",
            shellDepth: "#F0EEE6",
            shellDepthSoft: "#E8E6DC",
            shellFrame: "#D8D3C6",
            railBg: "#F5F2EA",
            railPanel: "#F0EEE6",
            railPanelStrong: "#E8E6DC",
            railCard: "#F7F4EC",
            railCardSelected: "#ECE7DC",
            boardBg: "#FAF9F5",
            boardPanel: "#F3F1E9",
            boardPanelMuted: "#F0EEE6",
            boardCard: "#F7F4EC",
            boardRow: "#F5F2EA",
            boardRowStrong: "#ECE7DC",
            borderDark: "#D8D3C6",
            borderLight: "#FEFCF8",
            borderFocus: "#D97757",
            gridLine: "#E2DDD0",
            textOnDark: "#141413",
            textOnDarkMuted: "#87867F",
            textOnLight: "#141413",
            textOnLightMuted: "#87867F",
            textOnLightSoft: "#B0AEA5",
            accentBronze: "#D97757",
            accentEmber: "#BF684A",
            accentReady: "#788C5D",
            accentReadyDeep: "#586744",
            accentOrange: "#B0AEA5",
            accentOrangeStrong: "#D97757",
            accentMuted: "#B0AEA5",
            accentSteel: "#8EA3BC",
            accentSteelBright: "#6A9BCC",
            accentCream: "#FAF9F5",
            accentSignal: "#D97757",
            accentSignalDeep: "#9D5238",
            accentPhosphor: "#788C5D",
            accentPhosphorSoft: "#99A98A",
            accentHermes: "#6A9BCC",
            accentHermesBright: "#8FB2D6",
            ambientRail: "#E8E6DC",
            ambientRailBright: "#D97757",
            ambientTrace: "#D8D3C6",
            ambientTraceBright: "#6A9BCC",
            danger: "#B85A45",
            warning: "#D97757",
            repositoryAccent: "#D97757",
            panelLead: "#D97757",
            panelLeadSoft: "#E8E6DC",
            panelRule: "#D8D3C6",
            themeControlFill: "#E8E6DC",
            themeControlTone: "#D97757",
            themeControlText: "#141413",
            railSignalPrimary: "#D97757",
            railSignalSecondary: "#6A9BCC",
            railSignalTertiary: "#788C5D",
            railPacketColor: "#D97757",
            railPulseColor: "#6A9BCC",
            railFieldTint: "#E8E6DC"
        },
        "claude_ink": {
            shellBg: "#141413",
            shellDepth: "#131314",
            shellDepthSoft: "#1B1A19",
            shellFrame: "#3D3D3A",
            railBg: "#141413",
            railPanel: "#1B1A19",
            railPanelStrong: "#23211F",
            railCard: "#1F1E1D",
            railCardSelected: "#2A2826",
            boardBg: "#141413",
            boardPanel: "#1A1918",
            boardPanelMuted: "#181716",
            boardCard: "#1F1E1D",
            boardRow: "#1C1B1A",
            boardRowStrong: "#262422",
            borderDark: "#3D3D3A",
            borderLight: "#66635D",
            borderFocus: "#D97757",
            gridLine: "#2A2826",
            textOnDark: "#FAF9F5",
            textOnDarkMuted: "#B0AEA5",
            textOnLight: "#FAF9F5",
            textOnLightMuted: "#D1CFC8",
            textOnLightSoft: "#B0AEA5",
            accentBronze: "#D97757",
            accentEmber: "#B85A45",
            accentReady: "#788C5D",
            accentReadyDeep: "#4D5B3C",
            accentOrange: "#87867F",
            accentOrangeStrong: "#D97757",
            accentMuted: "#66635D",
            accentSteel: "#557A9F",
            accentSteelBright: "#6A9BCC",
            accentCream: "#FAF9F5",
            accentSignal: "#D97757",
            accentSignalDeep: "#7E412E",
            accentPhosphor: "#788C5D",
            accentPhosphorSoft: "#96A786",
            accentHermes: "#6A9BCC",
            accentHermesBright: "#90B8E0",
            ambientRail: "#23211F",
            ambientRailBright: "#D97757",
            ambientTrace: "#262422",
            ambientTraceBright: "#6A9BCC",
            danger: "#E0876D",
            warning: "#D97757",
            repositoryAccent: "#D97757",
            panelLead: "#D97757",
            panelLeadSoft: "#3D3D3A",
            panelRule: "#B0AEA5",
            themeControlFill: "#23211F",
            themeControlTone: "#D97757",
            themeControlText: "#FAF9F5",
            railSignalPrimary: "#D97757",
            railSignalSecondary: "#6A9BCC",
            railSignalTertiary: "#788C5D",
            railPacketColor: "#D97757",
            railPulseColor: "#6A9BCC",
            railFieldTint: "#23211F"
        },
        "eva": {
            shellBg: "#000000",
            shellDepth: "#000000",
            shellDepthSoft: "#000000",
            shellFrame: "#000000",
            railBg: "#000000",
            railPanel: "#000000",
            railPanelStrong: "#000000",
            railCard: "#000000",
            railCardSelected: "#120404",
            boardBg: "#000000",
            boardPanel: "#000000",
            boardPanelMuted: "#000000",
            boardCard: "#000000",
            boardRow: "#000000",
            boardRowStrong: "#120404",
            borderDark: "#CC0000",
            borderLight: "#CC0000",
            borderFocus: "#FF0000",
            gridLine: "#220808",
            textOnDark: "#FFFFFF",
            textOnDarkMuted: "#E04040",
            textOnLight: "#FFFFFF",
            textOnLightMuted: "#E04040",
            textOnLightSoft: "#CC5555",
            accentBronze: "#CC0000",
            accentEmber: "#7A2020",
            accentReady: "#CC0000",
            accentReadyDeep: "#441111",
            accentOrange: "#CC0000",
            accentOrangeStrong: "#CC0000",
            accentMuted: "#772222",
            accentSteel: "#882222",
            accentSteelBright: "#DD4444",
            accentCream: "#FFFFFF",
            accentSignal: "#FF0000",
            accentSignalDeep: "#331010",
            accentPhosphor: "#CC0000",
            accentPhosphorSoft: "#CC0000",
            accentHermes: "#CC0000",
            accentHermesBright: "#CC0000",
            ambientRail: "#060000",
            ambientRailBright: "#CC0000",
            ambientTrace: "#0A0000",
            ambientTraceBright: "#CC0000",
            danger: "#FF0000",
            warning: "#FFAA00",
            repositoryAccent: "#CC0000",
            panelLead: "#CC0000",
            panelLeadSoft: "#0A0000",
            panelRule: "#CC0000",
            themeControlFill: "#220000",
            themeControlTone: "#CC0000",
            themeControlText: "#FFFFFF",
            railSignalPrimary: "#CC0000",
            railSignalSecondary: "#330000",
            railSignalTertiary: "#CC0000",
            railPacketColor: "#CC0000",
            railPulseColor: "#CC0000",
            railFieldTint: "#060000"
        }
    })

    readonly property var palette: palettes[currentThemeId]

    readonly property color shellBg: palette.shellBg
    readonly property color shellDepth: palette.shellDepth
    readonly property color shellDepthSoft: palette.shellDepthSoft
    readonly property color shellFrame: palette.shellFrame
    readonly property color railBg: palette.railBg
    readonly property color railPanel: palette.railPanel
    readonly property color railPanelStrong: palette.railPanelStrong
    readonly property color railCard: palette.railCard
    readonly property color railCardSelected: palette.railCardSelected
    readonly property color boardBg: palette.boardBg
    readonly property color boardPanel: palette.boardPanel
    readonly property color boardPanelMuted: palette.boardPanelMuted
    readonly property color boardCard: palette.boardCard
    readonly property color boardRow: palette.boardRow
    readonly property color boardRowStrong: palette.boardRowStrong
    readonly property color borderDark: palette.borderDark
    readonly property color borderLight: palette.borderLight
    readonly property color borderFocus: palette.borderFocus
    readonly property color gridLine: palette.gridLine
    readonly property color textOnDark: palette.textOnDark
    readonly property color textOnDarkMuted: palette.textOnDarkMuted
    readonly property color textOnLight: palette.textOnLight
    readonly property color textOnLightMuted: palette.textOnLightMuted
    readonly property color textOnLightSoft: palette.textOnLightSoft
    readonly property color accentBronze: palette.accentBronze
    readonly property color accentEmber: palette.accentEmber
    readonly property color accentReady: palette.accentReady
    readonly property color accentReadyDeep: palette.accentReadyDeep
    readonly property color accentOrange: palette.accentOrange
    readonly property color accentOrangeStrong: palette.accentOrangeStrong
    readonly property color accentMuted: palette.accentMuted
    readonly property color accentSteel: palette.accentSteel
    readonly property color accentSteelBright: palette.accentSteelBright
    readonly property color accentCream: palette.accentCream
    readonly property color accentSignal: palette.accentSignal
    readonly property color accentSignalDeep: palette.accentSignalDeep
    readonly property color accentPhosphor: palette.accentPhosphor
    readonly property color accentPhosphorSoft: palette.accentPhosphorSoft
    readonly property color accentHermes: palette.accentHermes
    readonly property color accentHermesBright: palette.accentHermesBright
    readonly property color ambientRail: palette.ambientRail
    readonly property color ambientRailBright: palette.ambientRailBright
    readonly property color ambientTrace: palette.ambientTrace
    readonly property color ambientTraceBright: palette.ambientTraceBright
    readonly property color danger: palette.danger
    readonly property color warning: palette.warning
    readonly property color repositoryAccent: palette.repositoryAccent
    readonly property color panelLead: palette.panelLead
    readonly property color panelLeadSoft: palette.panelLeadSoft
    readonly property color panelRule: palette.panelRule
    readonly property color themeControlFill: palette.themeControlFill
    readonly property color themeControlTone: palette.themeControlTone
    readonly property color themeControlText: palette.themeControlText
    readonly property color railSignalPrimary: palette.railSignalPrimary
    readonly property color railSignalSecondary: palette.railSignalSecondary
    readonly property color railSignalTertiary: palette.railSignalTertiary
    readonly property color railPacketColor: palette.railPacketColor
    readonly property color railPulseColor: palette.railPulseColor
    readonly property color railFieldTint: palette.railFieldTint
    readonly property color chromeGlass: palette.railFieldTint ? palette.railFieldTint : palette.boardPanelMuted

    readonly property int space2: 2
    readonly property int space3: 3
    readonly property int space4: 4
    readonly property int space5: 5
    readonly property int space6: 6
    readonly property int space8: 8
    readonly property int space9: 9
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

    readonly property int motionFast: 120
    readonly property int motionNormal: 180
    readonly property int motionSlow: 280
    readonly property int motionAmbient: 3600
    readonly property int motionDrift: 6200
    readonly property int startupBootCueMs: 4480
    readonly property int startupSplashCueMs: 5000
    readonly property int startupPhaseShort: 180
    readonly property int startupPhaseMedium: 340
    readonly property int startupPhaseLong: 620
    readonly property int startupReveal: 720
    readonly property int startupDismiss: 300
    readonly property int startupPulse: 560
    readonly property int startupTypewriter: 34
    readonly property bool ambientEnabled: true

    readonly property string displayFamily: "Roboto Condensed"
    readonly property string bodyFamily: "Noto Sans"
    readonly property string monoFamily: "JetBrainsMono Nerd Font Mono"
    readonly property string terminalMonoFamily: "DejaVu Sans Mono"

    function setTheme(themeId) {
        if (palettes[themeId]) {
            currentThemeId = themeId
        }
    }

    function withAlpha(colorValue, alphaValue) {
        return Qt.alpha(colorValue, alphaValue)
    }

    function sessionStateColor(toneKey) {
        switch (toneKey) {
        case "starting":
            return accentBronze
        case "idle":
            return accentReady
        case "thinking":
            return accentHermesBright
        case "running_tool":
            return accentHermes
        case "awaiting_approval":
            return accentSignal
        case "waiting_for_input":
            return warning
        case "backgrounded":
            return accentSteelBright
        case "exited":
            return accentMuted
        case "error":
        default:
            return danger
        }
    }
}
