// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning

ApplicationWindow {
    id: window
    width: Qt.platform.os === "android" ? Screen.width : 1024
    height: Qt.platform.os === "android" ? Screen.height : 768
    visible: true
    title: view.map.center + " zoom " + view.map.zoomLevel.toFixed(3)
           + " min " + view.map.minimumZoomLevel + " max " + view.map.maximumZoomLevel

    property var center: QtPositioning.coordinate(30.531389, 114.313833) //Tianditu

    MapView {
        id: view
        anchors.fill: parent
        map.plugin: map_plugin
        map.center: window.center
        map.zoomLevel: 8
        Component.onCompleted: {
            //print(map.minimumZoomLevel,map.maximumZoomLevel)
            //print(activeMapType.cameraCapabilities.minimumZoomLevel,activeMapType.cameraCapabilities.maximumZoomLevel)
            for(var i=0;i<map.supportedMapTypes.length;i++){
                print(i,map.supportedMapTypes[i])
            }
            map.minimumZoomLevel=map.activeMapType.cameraCapabilities.minimumZoomLevel
            map.maximumZoomLevel=map.activeMapType.cameraCapabilities.maximumZoomLevel
        }

        MouseHandler{
            parent: view.map
            anchors.fill: parent
            map: view.map
        }

        Marker {
            parent: view.map
            markerIndex: 1
            coordinate: window.center
            z: view.map.z+1
        }
    }
    Map {
        id: map_annotation
        parent: view.map
        anchors.fill: parent
        plugin: annotation_plugin
        copyrightsVisible: false
        center: view.map.center
        minimumFieldOfView: view.map.minimumFieldOfView
        maximumFieldOfView: view.map.maximumFieldOfView
        minimumTilt: view.map.minimumTilt
        maximumTilt: view.map.maximumTilt
        minimumZoomLevel: view.map.minimumZoomLevel
        maximumZoomLevel: view.map.maximumZoomLevel
        zoomLevel: view.map.zoomLevel
        tilt: view.map.tilt;
        bearing: view.map.bearing
        fieldOfView: view.map.fieldOfView
        z: view.map.z + 1
        visible: check_annotation.checked
        color: 'transparent'
    }

    Rectangle {
        anchors.topMargin: 1
        anchors.rightMargin: 1
        anchors.top: parent.top
        anchors.right: parent.right
        width: 100
        height: 25
        color: "white"
        border.width: 0
        radius: 2
        CheckBox {
            id: check_annotation
            anchors.centerIn: parent
            text: "显示注记" //Annotation
            checked: true
        }
    }

    MapToolBar {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 8
        anchors.bottomMargin: 16
        map: view.map
    }

    Plugin {
        id: map_plugin
        name: "tile"
        PluginParameter{
            name: "tile.mapping.name"
            value: "My Custom Map"
        }
        PluginParameter{
            name: "tile.mapping.tile_prefix"
            value: "tianditu_100-1-"
        }
        PluginParameter{
            name: "tile.mapping.minzoomlevel"
            value: 3
        }
        PluginParameter{
            name: "tile.mapping.maxzoomlevel"
            value: 14
        }
        PluginParameter{
            name: "tile.mapping.cache.directory"
            value: "D:\\git-repository\\tile\\example\\cache\\tianditu"
        }
        PluginParameter{
            name: "tile.mapping.cache.hierarchy"
            value: 0
        }
        PluginParameter{
            name: "tile.mapping.precachezoomlevel"
            value: view.map.zoomLevel
        }
    }

    Plugin {
        id: annotation_plugin
        name: "tile"
        PluginParameter{
            name: "tile.mapping.name"
            value: "Annotation"
        }
        PluginParameter{
            name: "tile.mapping.tile_prefix"
            value: "tianditu_100-2-"
        }
        PluginParameter{
            name: "tile.mapping.minzoomlevel"
            value: 3
        }
        PluginParameter{
            name: "tile.mapping.maxzoomlevel"
            value: 14
        }
        PluginParameter{
            name: "tile.mapping.cache.directory"
            value: "D:\\git-repository\\tile\\example\\cache\\tianditu"
        }
        PluginParameter{
            name: "tile.mapping.cache.hierarchy"
            value: 0
        }
        PluginParameter{
            name: "tile.mapping.precachezoomlevel"
            value: view.map.zoomLevel
        }
    }

    Shortcut {
        enabled: view.map.zoomLevel <  view.map.maximumZoomLevel
        sequence: StandardKey.ZoomIn
        onActivated: view.map.zoomLevel = Math.round(view.map.zoomLevel + 1)
    }
    Shortcut {
        enabled: view.map.zoomLevel > view.map.minimumZoomLevel
        sequence: StandardKey.ZoomOut
        onActivated: view.map.zoomLevel = Math.round(view.map.zoomLevel - 1)
    }
}
