# Offline stroage tile map plugin for QtLocation
<p align="center">
English | <a href="README_zh_CN.md">简体中文</a>
</p>

Offline tile map plugin based on WebMercator projection, using datum WGS84.

+ Demo-OSM
<div align=center>
  <img src="example/preview/osm.gif">
</div>
<p>（GIF file is large, may load slowly, if not displayed, view <a href="example/preview/osm.gif">osm.gif</a>）</p>

+ Support for **Windows**, **Linux**, **Android**, **MacOS** etc.
+ Support for **OSM**, **Google**, **Bing**, **Amap**, **Tianditu** and other offline tile maps.
+ Support map tilt, bearing etc.
+ Tile plugin bulit with Qt6.5.3 or higher(only CMake supported).
+ Offline maps directly use GPS latitude and longitude coordinates without conversion.
+ Only one map type is available(The map id is fixed to 1), please use multiple plugin instances for multiple map layers, refer to example.

## Parameters
| Parameter | Description |
|-------|-------|
| tile.mapping.name | Custom map name |
| tile.mapping.minzoomlevel | The minimum zoom level for offline tiles, default 0 |
| tile.mapping.maxzoomlevel | The maximum scaling level of offline tiles is 19 by default and largest 25 level |
| tile.mapping.precachezoomlevel | Initializes the zoom level of loaded offline tiles to increase the speed at which maps are first displayed. The default is 0 |
| tile.mapping.cache.directory | Cache directory of offline tiles |
| tile.mapping.cache.hierarchy | Directory hierarchy of an offline tile. The default value is 0<br> 0: cache/{prefix}{z}-{x}-{y}.{image}<br>1: cache/{z}/{x}-{y}.{image}<br>2: cache/{z}/{x}/{y}.{image}<br>3: cache/{z}/{y}/{x}.{image} |
| tile.mapping.tile_prefix | This parameter is used when hierarchy is 0 and is compatible with the default cache tile file name that displays QtLocation. The prefix is before zxy. for example:<br>osm_100-l-4-2-3.png -> tile_prefix:osm_100-l- |

## Get started

+ Clone the repository.

```SHELL
git clone --recursive https://github.com/mml-0104/tile.git
```

+ Build

```
git clone --recursive https://github.com/mml-0104/tile.git
cd tile
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=<YOUR_QT_SDK_DIR_PATH> -DCMAKE_BUILD_TYPE=Release -GNinja ../
cmake --build . --config Release --target all
```

or

Use your IDE (`Qt Creator` is recommended) to open the project and build. (The plugin and examples are built using **CMake**).
<div align=center>
  <img src="example/preview/qt_creator_project.png">
</div>

The **tile** plugin is now installed in the <YOUR_QT_SDK_DIR_PATH>/plugins/geoservices directory.


+ Run example

Decompress the test tile file in the example/cache directory to the current directory.

Modify the PluginParameter named **tile.mapping.cache.directory**  in the **osm.qml**  file using the decompression path described above.
```qml
//osm.qml
PluginParameter{
	name: "tile.mapping.cache.directory"
	value: "<YOUR_REPOSITORY_DIR>/tile/example/cache/osm"    
}
```

Modify the macro definition switch test tile set in main.cpp
```c++
//main.cpp
#if 0 //0: osm test  1:tianditu test
    engine.load(QUrl(QStringLiteral("qrc:/tianditu.qml")));
#else
    engine.load(QUrl(QStringLiteral("qrc:/osm.qml")));
#endif
```

+ Compile the project. Then try to execute the `example` demo program.

+ Great! Now you are ready to use **tile** plugin in your qt project, good lucky.

## Use plugin in your Qt project
1. Download or use an existing offline map tile file，The directory hierarchy of the file needs to be the way the plugin supports it.

2. Find the latitude and longitude(Datum WGS84)  of any location within the offline map (preferably at or near the center point).

3. Load your offline map in your Qt project (qmake or cmake): 
```qml
Window {
    id: window
    width: Qt.platform.os === "android" ? Screen.width : 1024
    height: Qt.platform.os === "android" ? Screen.height : 768
    
    property var center: QtPositioning.coordinate(59.9485,10.7686) //Oslo

    MapView {
        id: view
        anchors.fill: parent
        map.plugin: map_plugin
        map.center: window.center
        map.zoomLevel: 3
    }

    Plugin {
        id: map_plugin
        name: "tile"
        PluginParameter{
            name: "tile.mapping.name"
            value: "Your Custom Map Name"
        }
        PluginParameter{
            name: "tile.mapping.minzoomlevel"
            value: <YOUR_OFFLINE_MAP_MINZOOMLEVEL>
        }
        PluginParameter{
            name: "tile.mapping.maxzoomlevel"
            value: <YOUR_OFFLINE_MAP_MAXZOOMLEVEL>
        }
        PluginParameter{
            name: "tile.mapping.cache.directory"
            value: <YOUR_OFFLINE_TILE_DIRECTORY_PATH>
        }
        PluginParameter{
            name: "tile.mapping.cache.hierarchy"
            value: <YOUR_OFFLINE_TILE_HIERARCHY>
        }
        PluginParameter{
            name: "tile.mapping.precachezoomlevel"
            value: view.map.zoomLevel
        }
    }
}
```
4. Depending on the type of application, decide whether to allow over zooming of the map。Use the following code to disable overzooming of the map：
```qml
MapView {
	id: view
	Component.onCompleted: {
		map.minimumZoomLevel=map.activeMapType.cameraCapabilities.minimumZoomLevel
		map.maximumZoomLevel=map.activeMapType.cameraCapabilities.maximumZoomLevel
}
```


5. Limit map boundaries in your QML application to ensure that map movement/zooming does not exceed the range of the offline tile, so as not to cause a poor user experience.

# Reference
[**qtlocation**: The srouce code for QtLocation](https://github.com/qt/qtlocation)

[**java_map_download**: Offline map downloader](https://gitcode.com/kurimuson/java_map_download/overview)

+ Donate
<div align=center>
  <img src="example/donate/Alipay.png">
  <img src="example/donate/Wechat.png">
</div>
<div align=center>
(Alipay)&nbsp;&nbsp;|&nbsp;&nbsp;(Wechat)</br>
♥Buy the author a cup of coffee♥
</div>
