pragma ComponentBehavior: Bound
import QtQuick 6.5

Item {
    id: root

    property color lineColor: "white"
    property int spanLength: 18
    property int spanHeight: 14

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        width: root.spanLength
        height: 1
        color: root.lineColor
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        width: 1
        height: root.spanHeight
        color: root.lineColor
    }

    Rectangle {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: root.spanLength
        height: 1
        color: root.lineColor
    }

    Rectangle {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: 1
        height: root.spanHeight
        color: root.lineColor
    }
}
