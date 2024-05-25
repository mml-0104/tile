# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_add_plugin(GeoServiceProviderFactoryTilePlugin
    OUTPUT_NAME qtgeoservices_tile
    CLASS_NAME GeoServiceProviderFactoryTile
    PLUGIN_TYPE geoservices
    geoserviceproviderplugin_tile.h geoserviceproviderplugin_tile.cpp
    geotiledmappingmanagerengine_tile.h geotiledmappingmanagerengine_tile.cpp
    geotilefetcher_tile.h geotilefetcher_tile.cpp
    geofiletilecache_tile.h geofiletilecache_tile.cpp
    geotiledmapreply_tile.h geotiledmapreply_tile.cpp
    geofiletilecache.h geofiletilecache.cpp
)

set_target_properties(GeoServiceProviderFactoryTilePlugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${INSTALL_PLUGINDIR}"
)

target_link_libraries(GeoServiceProviderFactoryTilePlugin PRIVATE
    Qt::Core
    Qt::LocationPrivate
    Qt::PositioningPrivate
    Qt::Concurrent
)

qt_add_resources(GeoServiceProviderFactoryTilePlugin "res"
    PREFIX "/"
    FILES
        tile_plugin.json
)

install(TARGETS GeoServiceProviderFactoryTilePlugin
    RUNTIME DESTINATION "${INSTALL_PLUGINDIR}"
    LIBRARY DESTINATION "${INSTALL_PLUGINDIR}"
)
