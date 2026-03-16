pragma ComponentBehavior: Bound
import QtQuick 6.5
import "../styles"

Canvas {
    id: root

    required property var stateDistribution

    implicitWidth: 28
    implicitHeight: 28

    Accessible.role: Accessible.Indicator
    Accessible.name: "Aggregate session state"

    onStateDistributionChanged: requestPaint()

    onPaint: {
        const ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)

        const dist = root.stateDistribution
        if (!dist || dist.length === 0) {
            // Empty ring
            ctx.beginPath()
            ctx.arc(width / 2, height / 2, width / 2 - 3, 0, Math.PI * 2)
            ctx.strokeStyle = Qt.alpha(HydraTheme.accentSteelBright, 0.2)
            ctx.lineWidth = 3
            ctx.stroke()
            return
        }

        let total = 0
        for (let i = 0; i < dist.length; ++i) {
            total += dist[i].count
        }

        if (total === 0) return

        const cx = width / 2
        const cy = height / 2
        const r = cx - 3
        let startAngle = -Math.PI / 2

        for (let i = 0; i < dist.length; ++i) {
            const fraction = dist[i].count / total
            const sweepAngle = fraction * Math.PI * 2
            const toneColor = HydraTheme.sessionStateColor(dist[i].tone)

            ctx.beginPath()
            ctx.arc(cx, cy, r, startAngle, startAngle + sweepAngle)
            ctx.strokeStyle = Qt.alpha(toneColor, 0.85)
            ctx.lineWidth = 3
            ctx.lineCap = "butt"
            ctx.stroke()

            startAngle += sweepAngle
        }
    }
}
