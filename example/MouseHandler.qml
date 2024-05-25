import QtQuick

MouseArea {
    acceptedButtons: Qt.RightButton
    property var map
    property real posX
    property real posY
    property bool pressed: false
    property bool bearMap: false
    property bool tiltMap: false
    onPressed: pressed = true
    onReleased: {
        pressed = false
        bearMap = false
        tiltMap = false
    }
    onPositionChanged: function(mouse){
        if(!pressed)
            return

        let deltaX = mouse.x-posX
        let deltaY = mouse.y-posY

        if(bearMap) {
            map.bearing += deltaX/5
            posX = mouse.x
        } else if(Math.abs(deltaX) >= 10) {
            bearMap = true
            posX = mouse.x
        }
        if(tiltMap) {
            map.tilt -=  deltaY/5
            posY = mouse.y
        } else if(Math.abs(deltaY) >= 10) {
            tiltMap = true
            posY = mouse.y
        }
    }
}
