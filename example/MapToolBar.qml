import QtQuick
import QtQuick.Controls

Column {
    property var map
    spacing: 10
    Rectangle {
        width: 30
        height: width
        color: "white"
        border.width: 0
        radius: 5
        Image {
            anchors.centerIn: parent
            source: "qrc:/images/aim.png"
        }
        TapHandler {
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            gesturePolicy: TapHandler.WithinBounds
            onSingleTapped: map.center = window.center
        }
        HoverHandler {
            id: aimHover
        }
        ToolTip {
            visible: aimHover.hovered
            text: "Locate"
        }
    }

    Rectangle {
        width: 30
        height: width
        color: "white"
        border.width: 0
        radius: width/2
        Image {
            scale: 0.9
            anchors.centerIn: parent
            source: "qrc:/images/compass.png"
            rotation: -map.bearing
        }
        TapHandler {
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            gesturePolicy: TapHandler.WithinBounds
            onSingleTapped: {
                map.bearing = 0
                map.tilt = 0
            }
        }
        HoverHandler {
            id: bearHover
        }
        ToolTip {
            visible: bearHover.hovered
            text: "North"
        }
    }
    Column{
        spacing: 3
        Rectangle {
            width: 30
            height: width
            color: "white"
            border.width: 0
            radius: 5
            Text {
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: qsTr("+")
                font.pixelSize: 20
            }
            TapHandler {
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                gesturePolicy: TapHandler.WithinBounds
                onSingleTapped: map.zoomLevel += 1
            }
            HoverHandler {
                id: zoomOutHover
            }
            ToolTip {
                visible: zoomOutHover.hovered
                text: "Zoom Out"
            }
        }
        Rectangle {
            width: 30
            height: width
            color: "white"
            border.width: 0
            radius: 5
            Text {
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: qsTr("-")
                font.pixelSize: 20
            }
            TapHandler {
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                gesturePolicy: TapHandler.WithinBounds
                onSingleTapped: map.zoomLevel -= 1
            }
            HoverHandler {
                id: zoomInHover
            }
            ToolTip {
                visible: zoomInHover.hovered
                text: "Zoom In"
            }
        }
    }
}
