
cmake_minimum_required(VERSION 3.16)
project(offline_map LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/location/offline_map")

find_package(Qt6 COMPONENTS Core Gui Qml Location)

qt_add_executable(offline_map WIN32 MACOSX_BUNDLE
    main.cpp
)

target_link_libraries(offline_map PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Location
    Qt::Qml
)

# Resources:
set(qml_resource_files
    "osm.qml"
    "tianditu.qml"
    "MapToolBar.qml"
    "MouseHandler.qml"
    "Marker.qml"
    "images/aim.png"
    "images/compass.png"
    "images/marker.png"
)

qt6_add_resources(offline_map "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)

install(TARGETS offline_map
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
