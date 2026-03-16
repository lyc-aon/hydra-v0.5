pragma ComponentBehavior: Bound
import QtQuick 6.5
import "../styles"

Item {
    id: root

    property bool active: true
    property int phase: -1

    signal dismissed()

    visible: active || fadeOutAnim.running

    readonly property string labelFull: "NOUS RESEARCH HACKATHON 2026"
    readonly property int labelCharCount: Math.min(labelFull.length, internal.typedLabelChars)
    readonly property int cueDurationMs: HydraTheme.startupSplashCueMs
    readonly property int dismissDurationMs: 420
    readonly property var phaseIntervals: [260, 1500, 1450, 720, cueDurationMs - dismissDurationMs - 3930]
    readonly property int typewriterWindowMs: Math.round(phaseIntervals[2] * 0.8)
    readonly property int typewriterIntervalMs: Math.max(28, Math.round(typewriterWindowMs / Math.max(1, labelFull.length)))
    readonly property int glowPulseHalfMs: Math.round(phaseIntervals[3] / 2)

    readonly property color nousBlue: "#2E7AE6"
    readonly property color nousBlueDark: "#1B5DBF"
    readonly property color nousBlueFaint: "#C8DDFB"

    QtObject {
        id: internal
        property real backdropOpacity: 1
        property real logoOpacity: 0
        property real logoScale: 1.15
        property real logoRotation: -8
        property real glowOpacity: 0
        property real glowScale: 0.6
        property int typedLabelChars: 0
        property real labelOpacity: 0
        property real fadeOut: 1.0
        property real lineWidth: 0
        property real lineOpacity: 0
    }

    Timer {
        id: phaseTimer
        interval: 100
        repeat: false
        onTriggered: root.advancePhase()
    }

    Timer {
        id: typewriterTimer
        interval: root.typewriterIntervalMs
        repeat: true
        running: false
        onTriggered: {
            if (internal.typedLabelChars < root.labelFull.length) {
                internal.typedLabelChars++
            } else {
                typewriterTimer.stop()
            }
        }
    }

    ParallelAnimation {
        id: logoRevealAnim
        NumberAnimation {
            target: internal; property: "logoOpacity"
            from: 0; to: 1; duration: Math.round(root.phaseIntervals[1] * 0.88)
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            target: internal; property: "logoScale"
            from: 1.15; to: 1.0; duration: Math.round(root.phaseIntervals[1] * 0.94)
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            target: internal; property: "logoRotation"
            from: -8; to: 0; duration: Math.round(root.phaseIntervals[1] * 0.94)
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            target: internal; property: "glowOpacity"
            from: 0; to: 0.45; duration: Math.round(root.phaseIntervals[1] * 0.76)
            easing.type: Easing.OutQuad
        }
        NumberAnimation {
            target: internal; property: "glowScale"
            from: 0.6; to: 1.0; duration: Math.round(root.phaseIntervals[1] * 0.88)
            easing.type: Easing.OutCubic
        }
    }

    ParallelAnimation {
        id: labelRevealAnim
        NumberAnimation {
            target: internal; property: "labelOpacity"
            from: 0; to: 1; duration: Math.round(root.phaseIntervals[2] * 0.62)
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            target: internal; property: "lineWidth"
            from: 0; to: 1; duration: Math.round(root.phaseIntervals[2] * 0.62)
            easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            target: internal; property: "lineOpacity"
            from: 0; to: 1; duration: Math.round(root.phaseIntervals[2] * 0.48)
            easing.type: Easing.OutCubic
        }
    }

    SequentialAnimation {
        id: glowPulseAnim
        loops: Animation.Infinite
        NumberAnimation {
            target: internal; property: "glowOpacity"
            from: 0.45; to: 0.25; duration: root.glowPulseHalfMs
            easing.type: Easing.InOutSine
        }
        NumberAnimation {
            target: internal; property: "glowOpacity"
            from: 0.25; to: 0.45; duration: root.glowPulseHalfMs
            easing.type: Easing.InOutSine
        }
    }

    NumberAnimation {
        id: fadeOutAnim
        target: internal; property: "fadeOut"
        from: 1; to: 0; duration: root.dismissDurationMs
        easing.type: Easing.InCubic
        onFinished: {
            root.active = false
            root.dismissed()
        }
    }

    function start() {
        phase = -1
        internal.backdropOpacity = 1
        internal.logoOpacity = 0
        internal.logoScale = 1.15
        internal.logoRotation = -8
        internal.glowOpacity = 0
        internal.glowScale = 0.6
        internal.typedLabelChars = 0
        internal.labelOpacity = 0
        internal.fadeOut = 1.0
        internal.lineWidth = 0
        internal.lineOpacity = 0
        phaseTimer.interval = Math.round(HydraTheme.motionFast * 0.75)
        phaseTimer.start()
    }

    function advancePhase() {
        phase++
        if (phase >= phaseIntervals.length) {
            dismiss()
            return
        }
        enterPhase(phase)
        phaseTimer.interval = phaseIntervals[phase]
        phaseTimer.start()
    }

    function enterPhase(p) {
        switch (p) {
        case 0:
            HydraSounds.playSplash()
            break
        case 1:
            logoRevealAnim.start()
            break
        case 2:
            labelRevealAnim.start()
            typewriterTimer.start()
            break
        case 3:
            glowPulseAnim.start()
            break
        case 4:
            break
        }
    }

    function dismiss() {
        phaseTimer.stop()
        typewriterTimer.stop()
        glowPulseAnim.stop()
        if (!fadeOutAnim.running) {
            fadeOutAnim.start()
        }
    }

    function abort() {
        phaseTimer.stop()
        typewriterTimer.stop()
        logoRevealAnim.stop()
        labelRevealAnim.stop()
        glowPulseAnim.stop()
        fadeOutAnim.stop()
        active = false
    }

    Component.onCompleted: {
        if (active) {
            start()
        }
    }

    Keys.onPressed: function(event) {
        if (!active) {
            return
        }
        event.accepted = true
        if (event.key === Qt.Key_Escape) {
            dismiss()
        }
    }

    focus: active

    // -- White backdrop + event blocker --
    Rectangle {
        anchors.fill: parent
        color: "#FFFFFF"
        opacity: internal.backdropOpacity * internal.fadeOut

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
        }
    }

    // -- Glow behind logo --
    Rectangle {
        id: glow
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -40
        width: 360
        height: 360
        radius: 180
        color: "transparent"
        opacity: internal.fadeOut
        visible: root.phase >= 1

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: Qt.rgba(0.18, 0.48, 0.90, internal.glowOpacity * 0.25)
            scale: internal.glowScale
        }
        Rectangle {
            anchors.centerIn: parent
            width: 280
            height: 280
            radius: 140
            color: Qt.rgba(0.18, 0.48, 0.90, internal.glowOpacity * 0.12)
            scale: internal.glowScale
        }
    }

    // -- Logo --
    Image {
        id: logoImage
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -40
        width: 240
        height: 240
        source: Qt.resolvedUrl("../assets/nous_logo.png")
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: true
        opacity: internal.logoOpacity * internal.fadeOut
        scale: internal.logoScale
        rotation: internal.logoRotation
        visible: root.phase >= 1
    }

    // -- Text block --
    Column {
        id: textBlock
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: logoImage.bottom
        anchors.topMargin: 32
        spacing: 10
        opacity: internal.fadeOut
        visible: root.phase >= 2

        // Decorative line above text
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 280 * internal.lineWidth
            height: 2
            radius: 1
            color: root.nousBlue
            opacity: internal.lineOpacity * 0.6
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.labelFull.substring(0, root.labelCharCount)
            color: root.nousBlue
            font.family: HydraTheme.monoFamily
            font.pixelSize: 18
            font.bold: true
            font.letterSpacing: 4
            opacity: internal.labelOpacity

            Rectangle {
                anchors.left: parent.right
                anchors.leftMargin: 2
                anchors.verticalCenter: parent.verticalCenter
                width: 2
                height: parent.font.pixelSize
                color: root.nousBlue
                visible: root.labelCharCount < root.labelFull.length
                opacity: cursorBlink.running ? (cursorBlink.cursorOn ? 1 : 0) : 1

                Timer {
                    id: cursorBlink
                    property bool cursorOn: true
                    interval: 350
                    repeat: true
                    running: root.phase >= 2 && root.phase < 4
                    onTriggered: cursorOn = !cursorOn
                }
            }
        }

        // Decorative line below text
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 280 * internal.lineWidth
            height: 2
            radius: 1
            color: root.nousBlue
            opacity: internal.lineOpacity * 0.6
        }
    }

    // -- Subtle radial scanlines for texture --
    Repeater {
        model: root.phase >= 1 ? Math.ceil(root.height / 6) : 0
        Rectangle {
            required property int index
            x: 0
            y: index * 6
            width: root.width
            height: 1
            color: Qt.rgba(0.18, 0.48, 0.90, 0.018)
            visible: root.phase >= 1
            opacity: internal.backdropOpacity * internal.fadeOut
        }
    }
}
