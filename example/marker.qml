import QtQuick
import QtLocation

MapQuickItem {
    id: marker

    property string name
    property int markerIndex
    signal markerTapped();

    anchorPoint.x: image.width/2
    anchorPoint.y: image.height

    HoverHandler {
        id: hoverHandler
    }

    TapHandler {
        id: tapHandler
        acceptedButtons: Qt.LeftButton
        gesturePolicy: TapHandler.WithinBounds
        onTapped: markerTapped()
    }

    sourceItem: Image {
        id: image
//! [mqi-anchor]
        source: "qrc:/images/marker.png"
        opacity: hoverHandler.hovered ? 0.6 : 1.0

        Text{
            id: number
            y: image.height/10
            width: image.width
            color: "white"
            font.bold: true
            font.pixelSize: 14
            horizontalAlignment: Text.AlignHCenter
            text: String(markerIndex)
        }
//! [mqi-closeimage]
    }
}
