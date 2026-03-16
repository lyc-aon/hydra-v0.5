pragma ComponentBehavior: Bound
import QtQuick 6.5
import "../styles"

Item {
    id: root

    property bool active: true
    property int phase: -1

    signal dismissed()

    visible: active || dismissAnim.running

    readonly property string titleFull: "HYDRA V2"
    readonly property string subtitleFull: "ORCHESTRATION CONSOLE"
    readonly property int titleCharCount: Math.min(titleFull.length, internal.typedTitleChars)
    readonly property int subtitleCharCount: Math.min(subtitleFull.length, internal.typedSubtitleChars)
    readonly property int cueDurationMs: HydraTheme.startupBootCueMs
    readonly property int dismissDurationMs: 360
    readonly property var phaseIntervals: [220, 760, 950, 900, 650, cueDurationMs - dismissDurationMs - 3480]
    readonly property int typewriterWindowMs: Math.round(phaseIntervals[3] * 0.85)
    readonly property int typewriterIntervalMs: Math.max(24, Math.round(typewriterWindowMs / Math.max(1, titleFull.length + subtitleFull.length)))
    readonly property int bootLineRevealMs: Math.round(phaseIntervals[4] * 0.45)
    readonly property int glowPulseHalfMs: Math.round(phaseIntervals[5] / 2)
    readonly property int interferencePauseMs: 180
    readonly property int interferenceTravelMs: Math.max(280, phaseIntervals[2] - interferencePauseMs)

    readonly property var bootLines: [
        "[OK] TMUX BRIDGE",
        "[OK] SESSION STORE",
        "[OK] PROVIDER CATALOG"
    ]

    QtObject {
        id: internal
        property int typedTitleChars: 0
        property int typedSubtitleChars: 0
        property real sweepProgress: 0
        property real logoOpacity: 0
        property real logoScale: 0.85
        property real glowOpacity: 0
        property real interferenceY: -20
        property bool interferenceVisible: false
        property real dismissOpacity: 1.0
        property real bootLineOpacity0: 0
        property real bootLineOpacity1: 0
        property real bootLineOpacity2: 0
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
            if (internal.typedTitleChars < root.titleFull.length) {
                internal.typedTitleChars++
            } else if (internal.typedSubtitleChars < root.subtitleFull.length) {
                internal.typedSubtitleChars++
            } else {
                typewriterTimer.stop()
            }
        }
    }

    Timer {
        id: bootLine0Timer
        interval: 0; repeat: false
        onTriggered: bootLine0Anim.start()
    }
    Timer {
        id: bootLine1Timer
        interval: Math.round(root.phaseIntervals[4] * 0.34); repeat: false
        onTriggered: bootLine1Anim.start()
    }
    Timer {
        id: bootLine2Timer
        interval: Math.round(root.phaseIntervals[4] * 0.68); repeat: false
        onTriggered: bootLine2Anim.start()
    }

    NumberAnimation {
        id: bootLine0Anim
        target: internal; property: "bootLineOpacity0"
        from: 0; to: 1; duration: root.bootLineRevealMs
    }
    NumberAnimation {
        id: bootLine1Anim
        target: internal; property: "bootLineOpacity1"
        from: 0; to: 1; duration: root.bootLineRevealMs
    }
    NumberAnimation {
        id: bootLine2Anim
        target: internal; property: "bootLineOpacity2"
        from: 0; to: 1; duration: root.bootLineRevealMs
    }

    NumberAnimation {
        id: sweepAnim
        target: internal; property: "sweepProgress"
        from: 0; to: 1; duration: root.phaseIntervals[1]
        easing.type: Easing.InOutQuad
    }

    ParallelAnimation {
        id: logoRevealAnim
        NumberAnimation {
            target: internal; property: "logoOpacity"
            from: 0; to: 1; duration: Math.round(root.phaseIntervals[2] * 0.78)
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            target: internal; property: "logoScale"
            from: 0.85; to: 1.0; duration: Math.round(root.phaseIntervals[2] * 0.9)
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            target: internal; property: "glowOpacity"
            from: 0; to: 0.55; duration: root.phaseIntervals[2]
            easing.type: Easing.OutCubic
        }
    }

    SequentialAnimation {
        id: interferenceAnim
        PauseAnimation { duration: root.interferencePauseMs }
        PropertyAction { target: internal; property: "interferenceVisible"; value: true }
        NumberAnimation {
            target: internal; property: "interferenceY"
            from: -20; to: root.height + 20; duration: root.interferenceTravelMs
            easing.type: Easing.Linear
        }
        PropertyAction { target: internal; property: "interferenceVisible"; value: false }
    }

    NumberAnimation {
        id: dismissAnim
        target: internal; property: "dismissOpacity"
        from: 1; to: 0; duration: root.dismissDurationMs
        easing.type: Easing.InCubic
        onFinished: {
            root.active = false
            root.dismissed()
        }
    }

    SequentialAnimation {
        id: glowPulseAnim
        loops: Animation.Infinite
        NumberAnimation {
            target: internal; property: "glowOpacity"
            from: 0.55; to: 0.35; duration: root.glowPulseHalfMs
            easing.type: Easing.InOutSine
        }
        NumberAnimation {
            target: internal; property: "glowOpacity"
            from: 0.35; to: 0.55; duration: root.glowPulseHalfMs
            easing.type: Easing.InOutSine
        }
    }

    function start() {
        phase = -1
        internal.typedTitleChars = 0
        internal.typedSubtitleChars = 0
        internal.sweepProgress = 0
        internal.logoOpacity = 0
        internal.logoScale = 0.85
        internal.glowOpacity = 0
        internal.interferenceY = -20
        internal.interferenceVisible = false
        internal.dismissOpacity = 1.0
        internal.bootLineOpacity0 = 0
        internal.bootLineOpacity1 = 0
        internal.bootLineOpacity2 = 0
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
            HydraSounds.playBoot()
            break
        case 1:
            sweepAnim.start()
            break
        case 2:
            logoRevealAnim.start()
            interferenceAnim.start()
            break
        case 3:
            typewriterTimer.start()
            break
        case 4:
            bootLine0Timer.start()
            bootLine1Timer.start()
            bootLine2Timer.start()
            break
        case 5:
            glowPulseAnim.start()
            break
        }
    }

    function dismiss() {
        phaseTimer.stop()
        typewriterTimer.stop()
        glowPulseAnim.stop()
        if (!dismissAnim.running) {
            dismissAnim.start()
        }
    }

    function abort() {
        phaseTimer.stop()
        typewriterTimer.stop()
        bootLine0Timer.stop()
        bootLine1Timer.stop()
        bootLine2Timer.stop()
        bootLine0Anim.stop()
        bootLine1Anim.stop()
        bootLine2Anim.stop()
        sweepAnim.stop()
        logoRevealAnim.stop()
        interferenceAnim.stop()
        glowPulseAnim.stop()
        dismissAnim.stop()
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

    Rectangle {
        id: backdrop
        anchors.fill: parent
        color: "#000000"
        opacity: internal.dismissOpacity

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
        }

        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 1.2
            height: parent.height * 1.2
            rotation: 45
            gradient: Gradient {
                GradientStop { position: 0.0; color: "transparent" }
                GradientStop { position: 0.5; color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.03) }
                GradientStop { position: 1.0; color: "transparent" }
            }
            visible: root.phase >= 1
        }
    }

    Item {
        id: scanlineLayer
        anchors.fill: parent
        opacity: internal.dismissOpacity
        visible: root.phase >= 1
        clip: true

        Item {
            width: parent.width
            height: parent.height * internal.sweepProgress
            clip: true

            Repeater {
                model: Math.ceil(root.height / 3)
                Rectangle {
                    required property int index
                    x: 0
                    y: index * 3
                    width: scanlineLayer.width
                    height: 1
                    color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.04)
                }
            }
        }
    }

    Item {
        id: logoContainer
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -60
        width: 180
        height: 180
        opacity: internal.dismissOpacity
        visible: root.phase >= 2

        Rectangle {
            id: glowRect
            anchors.centerIn: parent
            width: 260
            height: 260
            radius: 130
            color: "transparent"
            border.width: 0

            Rectangle {
                anchors.fill: parent
                radius: parent.radius
                color: HydraTheme.withAlpha(HydraTheme.accentBronze, internal.glowOpacity * 0.3)
            }

            Rectangle {
                anchors.centerIn: parent
                width: 220
                height: 220
                radius: 110
                color: HydraTheme.withAlpha(HydraTheme.accentSignal, internal.glowOpacity * 0.15)
            }
        }

        HydraLogo {
            id: logoImage
            anchors.centerIn: parent
            logoSize: 180
            tintColor: HydraTheme.accentBronze
            opacity: internal.logoOpacity
            scale: internal.logoScale
        }
    }

    Column {
        id: textBlock
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: logoContainer.bottom
        anchors.topMargin: HydraTheme.space24
        spacing: HydraTheme.space6
        opacity: internal.dismissOpacity
        visible: root.phase >= 3

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.titleFull.substring(0, root.titleCharCount)
            color: HydraTheme.textOnDark
            font.family: HydraTheme.monoFamily
            font.pixelSize: 28
            font.bold: true
            font.letterSpacing: 6

            Rectangle {
                anchors.left: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: 2
                height: parent.font.pixelSize
                color: HydraTheme.accentBronze
                visible: root.titleCharCount < root.titleFull.length
                         || (root.phase === 3 && root.subtitleCharCount < root.subtitleFull.length)
                opacity: cursorBlink.running ? (cursorBlink.cursorVisible ? 1 : 0) : 1

                Timer {
                    id: cursorBlink
                    property bool cursorVisible: true
                    interval: 400
                    repeat: true
                    running: root.phase >= 3 && root.phase < 5
                    onTriggered: cursorVisible = !cursorVisible
                }
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.subtitleFull.substring(0, root.subtitleCharCount)
            color: HydraTheme.withAlpha(HydraTheme.textOnDarkMuted, 0.7)
            font.family: HydraTheme.monoFamily
            font.pixelSize: 13
            font.letterSpacing: 4
            visible: root.subtitleCharCount > 0
        }
    }

    Column {
        id: statusBlock
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: textBlock.bottom
        anchors.topMargin: HydraTheme.space18
        spacing: HydraTheme.space4
        opacity: internal.dismissOpacity
        visible: root.phase >= 4

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.bootLines[0]
            color: HydraTheme.withAlpha(HydraTheme.accentReady, 0.8)
            font.family: HydraTheme.monoFamily
            font.pixelSize: 11
            font.letterSpacing: 2
            opacity: internal.bootLineOpacity0
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.bootLines[1]
            color: HydraTheme.withAlpha(HydraTheme.accentReady, 0.8)
            font.family: HydraTheme.monoFamily
            font.pixelSize: 11
            font.letterSpacing: 2
            opacity: internal.bootLineOpacity1
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.bootLines[2]
            color: HydraTheme.withAlpha(HydraTheme.accentReady, 0.8)
            font.family: HydraTheme.monoFamily
            font.pixelSize: 11
            font.letterSpacing: 2
            opacity: internal.bootLineOpacity2
        }
    }

    Rectangle {
        id: interferenceBar
        x: 0
        y: internal.interferenceY
        width: parent.width
        height: 2
        color: HydraTheme.withAlpha(HydraTheme.accentBronze, 0.25)
        visible: internal.interferenceVisible
        opacity: internal.dismissOpacity
    }

    Repeater {
        model: Math.ceil(root.height / 3)
        Rectangle {
            required property int index
            x: 0
            y: index * 3
            width: root.width
            height: 1
            color: HydraTheme.withAlpha(HydraTheme.textOnDark, 0.025)
            visible: root.phase >= 1
            opacity: internal.dismissOpacity
        }
    }
}
