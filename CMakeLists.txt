#***************************************************************************
# Project:  NextGIS common desctop libraries
# Purpose:  cmake script
# Author:   Dmitry Baryshnikov, bishop.dev@gmail.com
#***************************************************************************
#   Copyright (C) 2015-2018 NextGIS, info@nextgis.ru
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#***************************************************************************

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

project(ngstd)

# some init settings
set(CMAKE_COLOR_MAKEFILE ON)
# set path to additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

if(NOT DEFINED PACKAGE_VENDOR)
    set(PACKAGE_VENDOR nextgis)
endif()

add_definitions(-DPACKAGE_VENDOR=${PACKAGE_VENDOR})

if(OSX_FRAMEWORK AND (BUILD_SHARED_LIBS OR BUILD_STATIC_LIBS))
  message(FATAL_ERROR "Only OSX_FRAMEWORK key or any or both BUILD_SHARED_LIBS
                       and BUILD_STATIC_LIBS keys are permitted")
endif()

if(OSX_FRAMEWORK)
  set(INSTALL_LIB_DIR "Library/Frameworks" CACHE INTERNAL "Installation directory for libraries" FORCE)
  set(SKIP_INSTALL_HEADERS ON)
  set(SKIP_INSTALL_FILES ON)

  set(CMAKE_MACOSX_RPATH ON)
else()
    include(GNUInstallDirs)

    set(INSTALL_LIB_DIR ${CMAKE_INSTALL_LIBDIR} CACHE INTERNAL "Installation directory for libraries" FORCE)
    set(INSTALL_INC_DIR "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}" CACHE INTERNAL "Installation directory for headers" FORCE)
    set(INSTALL_SHARE_DIR "${CMAKE_INSTALL_DATADIR}/${PACKAGE_VENDOR}/${PROJECT_NAME}" CACHE INTERNAL "Installation directory for share data" FORCE)
    set(INSTALL_PKGCONFIG_DIR "${INSTALL_LIB_DIR}/pkgconfig" CACHE INTERNAL "Installation directory for pkgconfig (.pc) files" FORCE)
    set(INSTALL_TRANSLATIONS_DIR "${CMAKE_INSTALL_DATADIR}/translations" CACHE INTERNAL "Installation directory for translations" FORCE)
endif()

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(util)
check_version(NGLIB_MAJOR_VERSION NGLIB_MINOR_VERSION NGLIB_PATCH_NUMBER)
set(VERSION ${NGLIB_MAJOR_VERSION}.${NGLIB_MINOR_VERSION}.${NGLIB_PATCH_NUMBER})

if(OSX_FRAMEWORK)
    set(FRAMEWORK_VERSION ${NGLIB_MAJOR_VERSION})
endif()

report_version(${PROJECT_NAME} ${VERSION})

configure_file(cmake/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake IMMEDIATE @ONLY)

set(NGSTD_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(NGLIBS
    core
    framework
)

add_subdirectory(src)
add_subdirectory(translations)

# export (TARGETS ${EXPORT_TARGETS} FILE ${EXPORT_TARGET_NAME}-exports.cmake EXPORT_LINK_INTERFACE_LIBRARIES)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
