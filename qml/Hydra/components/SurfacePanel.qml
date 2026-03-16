pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Layouts 6.5
import "../styles"

Rectangle {
    id: root

    property color panelColor: HydraTheme.railPanel
    property color panelBorderColor: HydraTheme.borderDark
    property int panelRadius: HydraTheme.radius10
    property int contentMargin: HydraTheme.space10
    property int contentSpacing: HydraTheme.space8
    property bool showHexGrid: false
    readonly property bool ogSteamTheme: HydraTheme.currentThemeId === "og_steam"
    readonly property bool evaTheme: HydraTheme.currentThemeId === "eva"
    default property alias contentData: contentLayout.data

    color: panelColor
    border.width: 1
    border.color: panelBorderColor
    radius: panelRadius
    implicitHeight: contentLayout.implicitHeight + (contentMargin * 2)
    clip: true

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 2
        color: HydraTheme.withAlpha(HydraTheme.panelLead, root.ogSteamTheme ? 0.34 : 0.24)
    }

    Canvas {
        anchors.fill: parent
        anchors.margins: 1
        visible: root.showHexGrid && root.evaTheme
        opacity: 0.18
        antialiasing: false
        z: 0

        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            const side = 14
            const hexWidth = side * 2
            const hexHeight = Math.sqrt(3) * side
            const colStep = side * 1.5
            const rowStep = hexHeight
            ctx.lineWidth = 1
            ctx.lineJoin = "miter"
            ctx.strokeStyle = HydraTheme.withAlpha(HydraTheme.panelLeadSoft, 0.08)

            for (let row = -2; row * rowStep < height + hexHeight; ++row) {
                const y = row * rowStep
                for (let col = -2; col * colStep < width + hexWidth; ++col) {
                    const x = col * colStep
                    const offsetY = (col % 2 === 0) ? 0 : (hexHeight / 2)
                    const top = y + offsetY

                    ctx.beginPath()
                    ctx.moveTo(x + side * 0.5, top)
                    ctx.lineTo(x + side * 1.5, top)
                    ctx.lineTo(x + hexWidth, top + hexHeight * 0.5)
                    ctx.lineTo(x + side * 1.5, top + hexHeight)
                    ctx.lineTo(x + side * 0.5, top + hexHeight)
                    ctx.lineTo(x, top + hexHeight * 0.5)
                    ctx.closePath()
                    ctx.stroke()
                }
            }
        }

        onVisibleChanged: requestPaint()
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: HydraTheme.space10
        width: Math.max(42, parent.width * 0.22)
        height: 1
        color: HydraTheme.withAlpha(HydraTheme.panelLeadSoft, root.ogSteamTheme ? 0.42 : 0.32)
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: HydraTheme.space10
        anchors.topMargin: HydraTheme.space10
        width: 18
        height: 1
        color: HydraTheme.withAlpha(HydraTheme.panelRule, root.ogSteamTheme ? 0.34 : 0.24)
    }

    ColumnLayout {
        id: contentLayout

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: root.contentMargin
        spacing: root.contentSpacing
    }
}
