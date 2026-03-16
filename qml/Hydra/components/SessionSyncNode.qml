pragma ComponentBehavior: Bound
import QtQuick 6.5
import "../styles"

Item {
    id: root

    required property string sessionId
    required property string alias
    required property string stateTone
    required property string activityLabel
    required property bool approvalPending
    required property var timelineEntries
    property bool selected: false
    property bool denseMode: false

    signal sessionSelected(string sessionId)

    function flashRoute() {
        routeFlashAnim.restart()
    }

    readonly property int nodeSize: root.denseMode ? 50 : 60
    readonly property color stateColor: HydraTheme.sessionStateColor(root.stateTone)
    readonly property color activityColor: root.approvalPending
                                          ? HydraTheme.accentSignal
                                          : (root.stateTone === "error"
                                             ? HydraTheme.danger
                                             : (root.activityLabel === "ACTIVE"
                                                ? HydraTheme.accentHermesBright
                                                : (root.activityLabel === "INPUT"
                                                   ? HydraTheme.accentBronze
                                                   : root.stateColor)))
    readonly property string displayLetter: root.alias.length > 0
                                             ? root.alias.charAt(0).toUpperCase()
                                             : "?"
    readonly property bool activePulse: root.activityLabel === "ACTIVE"
                                        || root.activityLabel === "INPUT"
                                        || root.approvalPending

    implicitWidth: root.nodeSize + 12
    implicitHeight: root.nodeSize + 20

    Accessible.role: Accessible.Button
    Accessible.name: "Session " + root.alias + " " + root.stateTone + " " + root.activityLabel

    SequentialAnimation {
        id: routeFlashAnim
        NumberAnimation { target: haloRing; property: "opacity"; to: 1.0; duration: 110; easing.type: Easing.OutQuad }
        NumberAnimation { target: ringScale; property: "value"; to: 1.12; duration: 130; easing.type: Easing.OutQuad }
        NumberAnimation { target: haloRing; property: "opacity"; to: 0.18; duration: 220; easing.type: Easing.InOutSine }
        NumberAnimation { target: ringScale; property: "value"; to: 1.0; duration: 220; easing.type: Easing.InOutSine }
    }

    QtObject {
        id: ringScale
        property real value: 1.0
    }

    Item {
        id: nodeFrame
        width: root.nodeSize
        height: root.nodeSize
        anchors.horizontalCenter: parent.horizontalCenter
        scale: ringScale.value

        Rectangle {
            id: haloRing
            anchors.centerIn: parent
            width: parent.width + 12
            height: width
            radius: width / 2
            color: "transparent"
            border.width: 1
            border.color: HydraTheme.withAlpha(root.activityColor, 0.72)
            opacity: root.selected ? 0.46 : 0.18
        }

        Canvas {
            id: orbitCanvas
            anchors.fill: parent

            onPaint: {
                const ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)

                const cx = width / 2
                const cy = height / 2
                const radius = cx - 3

                const palette = {
                    idle: {segments: 2, arc: 0.9, lineWidth: root.selected ? 2.6 : 1.8},
                    thinking: {segments: 3, arc: 0.65, lineWidth: 2.1},
                    running_tool: {segments: 5, arc: 0.34, lineWidth: 2.4},
                    awaiting_approval: {segments: 1, arc: 1.7, lineWidth: 2.8},
                    error: {segments: 6, arc: 0.18, lineWidth: 2.0},
                    starting: {segments: 4, arc: 0.4, lineWidth: 2.0},
                    waiting_for_input: {segments: 2, arc: 0.24, lineWidth: 2.2}
                }

                const profile = palette[root.stateTone] || palette.idle
                for (let i = 0; i < profile.segments; ++i) {
                    const start = ((Math.PI * 2) / profile.segments) * i
                    const end = start + profile.arc
                    ctx.beginPath()
                    ctx.arc(cx, cy, radius, start, end)
                    ctx.strokeStyle = Qt.alpha(root.activityColor, i === 0 ? 0.95 : 0.72)
                    ctx.lineWidth = profile.lineWidth
                    ctx.stroke()
                }

                ctx.beginPath()
                ctx.arc(cx, cy, radius, 0, Math.PI * 2)
                ctx.strokeStyle = Qt.alpha(root.stateColor, 0.14)
                ctx.lineWidth = 1
                ctx.stroke()
            }
        }

        Connections {
            target: root

            function onStateToneChanged() { orbitCanvas.requestPaint() }
            function onActivityLabelChanged() { orbitCanvas.requestPaint() }
            function onApprovalPendingChanged() { orbitCanvas.requestPaint() }
            function onSelectedChanged() { orbitCanvas.requestPaint() }
        }

        Item {
            id: sweepIndicator
            anchors.fill: parent
            visible: !root.approvalPending && root.stateTone !== "idle"

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 2
                width: root.stateTone === "running_tool" ? 10 : 6
                height: root.stateTone === "running_tool" ? 8 : 5
                radius: width / 2
                color: Qt.alpha(root.activityColor, 0.95)
                opacity: root.activePulse ? 1.0 : 0.55
            }
        }

        Rectangle {
            anchors.centerIn: parent
            width: root.nodeSize - (root.denseMode ? 16 : 18)
            height: width
            radius: width / 2
            color: Qt.alpha(root.stateColor, root.stateTone === "error" ? 0.22 : 0.18)
            border.width: 1
            border.color: Qt.alpha(root.activityColor, root.selected ? 0.74 : 0.34)
            scale: root.activePulse ? 1.02 : 1.0
        }

        Text {
            anchors.centerIn: parent
            text: root.displayLetter
            color: HydraTheme.textOnDark
            font.family: HydraTheme.monoFamily
            font.pixelSize: root.denseMode ? 13 : 15
            font.bold: true
            opacity: root.stateTone === "error" ? 0.9 : 1.0
        }

        Rectangle {
            anchors.fill: parent
            radius: width / 2
            color: "transparent"
            border.width: root.selected ? 2 : 1
            border.color: root.selected
                          ? HydraTheme.withAlpha(HydraTheme.accentBronze, 0.76)
                          : HydraTheme.withAlpha(root.stateColor, 0.18)
        }
    }

    Canvas {
        id: timelineBand
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        width: root.nodeSize - 4
        height: 7

        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            const entries = root.timelineEntries || []
            if (!entries.length) {
                ctx.fillStyle = Qt.alpha(root.stateColor, 0.2)
                ctx.fillRect(0, 2, width, height - 4)
                return
            }

            let totalDuration = 0
            for (let i = 0; i < entries.length; ++i) {
                totalDuration += (entries[i].durationSecs || 60)
            }
            if (totalDuration <= 0) totalDuration = 1

            let xPos = width
            for (let i = 0; i < entries.length; ++i) {
                const duration = entries[i].durationSecs || 60
                const segWidth = Math.max(2, Math.round((duration / totalDuration) * width))
                xPos -= segWidth
                const tone = entries[i].stateTone || root.stateTone
                const color = HydraTheme.sessionStateColor(tone)
                ctx.fillStyle = Qt.alpha(color, i === 0 ? 0.95 : 0.58)
                ctx.fillRect(Math.max(0, xPos), 1, segWidth, height - 2)
            }
        }
    }

    TapHandler {
        onTapped: root.sessionSelected(root.sessionId)
    }
}
