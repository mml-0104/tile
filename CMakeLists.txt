# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(tile LANGUAGES CXX
	DESCRIPTION "Qt Location - Offline tile map plugin"
)

if(NOT DEFINED INSTALL_PLUGINDIR)
    set(INSTALL_PLUGINDIR ${CMAKE_PREFIX_PATH})
endif()

set(INSTALL_PLUGINDIR "${INSTALL_PLUGINDIR}/plugins/geoservices")

find_package(Qt6 REQUIRED Core Positioning Location Concurrent)

qt_standard_project_setup()

add_subdirectory(plugin)
add_subdirectory(example)
