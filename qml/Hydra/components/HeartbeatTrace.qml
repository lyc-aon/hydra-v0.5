pragma ComponentBehavior: Bound
import QtQuick 6.5
import "../styles"

Item {
    id: root

    property string mode: "idle"  // "idle", "active", "error"
    property color traceColor: HydraTheme.accentReady
    property color errorColor: HydraTheme.danger

    implicitHeight: 48
    implicitWidth: 200

    readonly property color _drawColor: root.mode === "error" ? root.errorColor : root.traceColor
    readonly property int _segmentCount: 20
    readonly property real _activePeakHeight: Math.max(8, root.height * 0.62)
    readonly property real _idleHeight: 2

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        height: 1
        color: Qt.alpha(root._drawColor, root.mode === "error" ? 0.55 : 0.22)
    }

    Row {
        id: traceRow
        anchors.fill: parent
        anchors.topMargin: 6
        anchors.bottomMargin: 6
        spacing: Math.max(2, Math.floor(root.width / 90))

        Repeater {
            model: root._segmentCount

            Rectangle {
                required property int index

                readonly property real normalizedIndex: root._segmentCount <= 1
                                                        ? 0
                                                        : index / (root._segmentCount - 1)
                readonly property real activeEnvelope: Math.exp(
                                                           -Math.pow((normalizedIndex - 0.52) / 0.16, 2))
                readonly property real segmentHeight: root.mode === "error"
                                                      ? root._idleHeight
                                                      : (root.mode === "active"
                                                         ? Math.max(root._idleHeight,
                                                                    root._activePeakHeight * activeEnvelope)
                                                         : root._idleHeight)

                width: Math.max(4, Math.floor((root.width - traceRow.spacing * (root._segmentCount - 1))
                                              / root._segmentCount))
                height: segmentHeight
                radius: width / 2
                anchors.verticalCenter: parent.verticalCenter
                color: Qt.alpha(root._drawColor,
                                root.mode === "active"
                                ? (0.24 + activeEnvelope * 0.72)
                                : (root.mode === "error" ? 0.72 : 0.28))
            }
        }
    }

    Rectangle {
        visible: root.mode === "error"
        width: 2
        height: Math.max(12, root.height * 0.4)
        radius: 1
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        color: Qt.alpha(root.errorColor, 0.9)
    }
}
