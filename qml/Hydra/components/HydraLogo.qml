pragma ComponentBehavior: Bound
import QtQuick 6.5
import QtQuick.Effects
import "../styles"

Item {
    id: root
    property real logoSize: 20
    property color tintColor: HydraTheme.accentBronze
    width: logoSize
    height: logoSize

    Image {
        id: sourceImage
        anchors.fill: parent
        source: "../assets/hydra_logo_mono.png"
        fillMode: Image.PreserveAspectFit
        smooth: true
        antialiasing: true
        mipmap: true
        visible: false
    }

    MultiEffect {
        anchors.fill: parent
        source: sourceImage
        colorization: 1.0
        colorizationColor: root.tintColor
    }
}
