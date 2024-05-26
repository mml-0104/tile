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

    property var center: QtPositioning.coordinate(59.9485,10.7686) //Osm

    MapView {
        id: view
        anchors.fill: parent
        map.plugin: map_plugin
        map.center: window.center
        map.zoomLevel: 3
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
            value: "osm_100-l-1-"
        }
        PluginParameter{
            name: "tile.mapping.minzoomlevel"
            value: 5
        }
        PluginParameter{
            name: "tile.mapping.maxzoomlevel"
            value: 14
        }
        PluginParameter{
            name: "tile.mapping.cache.directory"
            value: "D:\\git-repository\\tile\\example\\cache\\osm"
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
