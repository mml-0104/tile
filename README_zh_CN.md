# QtLocation离线瓦片地图插件

基于Web墨卡托投影的离线瓦片地图插件，使用WGS84坐标系。

+ 演示程序-Tianditu（[天地图](https://www.tianditu.gov.cn/)）
<div align=center>
  <img src="example/preview/tianditu.gif">
</div>
<p>（GIF文件较大，可能加载较慢，如无法显示，查看 <a href="example/preview/tianditu.gif">tianditu.gif</a>）</p>


+ 支持**Windows**, **Linux**, **Android**, **MacOS**等。
+ 支持**OSM**, **Google**, **Bing**, **Amap**, **Tianditu**以及其他离线瓦片地图。
+ 支持地图倾斜、方位转动。
+ 插件使用Qt6.5.3或更高版本构建(仅支持CMake)。
+ 离线地图直接使用GPS经纬度坐标，无需转换。
+ 只有一种地图类型可用(地图id固定为1)，多个地图图层时请使用多个插件实例，参考示例example。

## 插件参数
| Parameter | Description |
|-------|-------|
| tile.mapping.name | 自定义地图名称 |
| tile.mapping.minzoomlevel | 离线地图的最小缩放级别，默认0 |
| tile.mapping.maxzoomlevel | 离线地图的最大缩放级别，默认为19，最大到25 |
| tile.mapping.precachezoomlevel | 初始化加载离线地图的缩放级别，以提高地图首次显示的速度。默认值为0 |
| tile.mapping.cache.directory | 离线地图瓦片的缓存目录 |
| tile.mapping.cache.hierarchy | 离线地图瓦片的目录层次结构. 默认值为0 <br> 0: cache/{prefix}{z}-{x}-{y}.{image} <br> 1: cache/{z}/{x}-{y}.{image} <br> 2: cache/{z}/{x}/{y}.{image} <br> 3: cache/{z}/{y}/{x}.{image} |
| tile.mapping.tile_prefix | 当hierarchy为0时使用此参数，用于兼容显示QtLocation的默认缓存瓦片文件，zxy之前的前缀. 例如:<br>osm_100-l-4-2-3.png -> tile_prefix:osm_100-l- |

## 快速开始

+ 克隆此仓库。.

```SHELL
git clone --recursive https://github.com/mml-0104/tile.git
```

+ 构建

```
git clone --recursive https://github.com/mml-0104/tile.git
cd tile
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=<YOUR_QT_SDK_DIR_PATH> -DCMAKE_BUILD_TYPE=Release -GNinja ../
cmake --build . --config Release --target all
```

或者

使用IDE(推荐使用``Qt Creator``)打开项目并进行构建。(插件和示例使用**CMake**构建)。


<div align=center>
  <img src="example/preview/qt_creator_project.png">
</div>

**tile**插件现在安装在<YOUR_QT_SDK_DIR_PATH>/plugins/geoservices目录下。


+ 运行example

将example/cache目录下的测试瓦片文件解压到当前目录下。

在**tianditu.qml**中修改名为**tile.mapping.cache.directory**的PluginParameter，使用上述的解压路径。

```qml
//tianditu.qml
PluginParameter{
	name: "tile.mapping.cache.directory"
	value: "<YOUR_REPOSITORY_DIR>/tile/example/cache/tianditu"
}
```

修改main.cpp中的宏定义以测试
```c++
main.cpp
#if 1 //0: osm test  1:tianditu test
    engine.load(QUrl(QStringLiteral("qrc:/tianditu.qml")));
#else
    engine.load(QUrl(QStringLiteral("qrc:/osm.qml")));
#endif
```

+ 编译项目。然后尝试执行`example` 演示程序。

+ 非常好！现在你已经准备好在你的Qt项目中使用**tile**插件了，祝你好运。 

## 在你的Qt工程中使用插件

1. 下载或使用已有的离线地图瓦片文件，文件的目录层次结构需是插件支持的方式。

2. 找到离线地图范围内任意位置的（最好是中心点或附近点）的经纬度（WGS84坐标系）。

3. 在你的Qt工程（qmake或cmake）中加载你的离线地图：
```qml
Window {
    id: window
    width: Qt.platform.os === "android" ? Screen.width : 1024
    height: Qt.platform.os === "android" ? Screen.height : 768
    
    property var center: QtPositioning.coordinate(30.531389, 114.313833) //武汉市

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
4. 根据应用类型，决定是否允许地图过量缩放。使用以下代码禁用地图过量缩放：
```qml
MapView {
	id: view
	Component.onCompleted: {
		map.minimumZoomLevel=map.activeMapType.cameraCapabilities.minimumZoomLevel
		map.maximumZoomLevel=map.activeMapType.cameraCapabilities.maximumZoomLevel
}
```


5. 在你QML应用程序中限制地图的边界，保证地图移动/缩放时不超出离线瓦片的范围，以免造成用户体验不佳。

# 参考
[**qtlocation**: QtLocation源代码](https://github.com/qt/qtlocation)

[**java_map_download**: 离线地图下载器](https://gitcode.com/kurimuson/java_map_download/overview)

+ 打赏作者
<div align=center>
  <img src="example/donate/Alipay.png">
  <img src="example/donate/Wechat.png">
</div>
<div align=center>
(Alipay)&nbsp;&nbsp;|&nbsp;&nbsp;(Wechat)</br>
♥请作者喝杯咖啡♥
</div>
