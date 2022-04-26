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
    set(PACKAGE_VENDOR NextGIS)
endif()

if(NOT DEFINED PACKAGE_BUGREPORT)
    set(PACKAGE_BUGREPORT info@nextgis.com)
endif()

add_definitions(-DPACKAGE_VENDOR=${PACKAGE_VENDOR})

if(OSX_FRAMEWORK AND (BUILD_SHARED_LIBS OR BUILD_STATIC_LIBS))
  message(FATAL_ERROR "Only OSX_FRAMEWORK key or any or both BUILD_SHARED_LIBS
                       and BUILD_STATIC_LIBS keys are permitted")
endif()

if(OSX_FRAMEWORK)
    set(INSTALL_LIB_DIR "Library/Frameworks" CACHE INTERNAL "Installation directory for libraries" FORCE)
    set(INSTALL_CMAKECONF_DIR CMake CACHE INTERNAL "Installation directory for cmake config files" FORCE)
    set(SKIP_INSTALL_HEADERS ON)
    set(SKIP_INSTALL_FILES ON)

    set(CMAKE_MACOSX_RPATH ON)
else()
    include(GNUInstallDirs)

    set(INSTALL_BIN_DIR ${CMAKE_INSTALL_BINDIR} CACHE INTERNAL "Installation directory for executables" FORCE)
    set(INSTALL_LIB_DIR ${CMAKE_INSTALL_LIBDIR} CACHE INTERNAL "Installation directory for libraries" FORCE)
    set(INSTALL_INC_DIR "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}" CACHE INTERNAL "Installation directory for headers" FORCE)
    set(INSTALL_TRANSLATIONS_DIR "${CMAKE_INSTALL_DATADIR}/translations" CACHE INTERNAL "Installation directory for translations" FORCE)
    set(INSTALL_CMAKECONF_DIR ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/CMake CACHE INTERNAL "Installation directory for cmake config files" FORCE)
endif()

if(POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()

# Tell CMake to run moc when necessary:
# FIXME: Not working on Qt <= 5.5 and Ubuntu [trusty, xenial]
# set(CMAKE_AUTOMOC ON)

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

set(PACKAGE_NAME ${PROJECT_NAME})
string(TOUPPER ${PACKAGE_NAME} PACKAGE_UPPER_NAME)

option(WITH_BINDINGS "Build python bindings" OFF)

if(WITH_BINDINGS)
    find_package(Python3 COMPONENTS Interpreter Development)

    if(${Python3_FOUND})
        set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
        set(PYTHON_LIBRARY ${Python3_LIBRARIES})
        set(PYTHON_VERSION_MAJOR ${Python3_VERSION_MAJOR})
        set(PYTHON_VERSION_MINOR ${Python3_VERSION_MINOR})
    endif()

    if(OSX_FRAMEWORK)
        set(NGSTD_PYTHON_DIR "Library/Python/${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages/${PROJECT_NAME}")
    elseif(WIN32)
        set(NGSTD_PYTHON_DIR "${INSTALL_LIB_DIR}/Python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}/site-packages/${PROJECT_NAME}")
    else()
        set(NGSTD_PYTHON_DIR "lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/dist-packages/${PROJECT_NAME}")
    endif()
endif()

include(FindAnyProject)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(NGSTD_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Used in src and translations
set(NGLIBS
    core
    framework
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_anyproject(Qt5 REQUIRED COMPONENTS Core Widgets Gui Network Svg LinguistTools)
if(WITH_ZLIB_EXTERNAL)
    get_target_property(ZLIB_IMPORTED_LOCATION ${ZLIB_LIBRARIES} IMPORTED_LOCATION_RELEASE)
    get_target_property(RCC_PATH Qt5::lrelease IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR ${RCC_PATH} PATH)
    if(WIN32)
        get_filename_component(PATH_DIR ${ZLIB_IMPORTED_LOCATION} PATH)
        set(ENV{PATH} "$ENV{PATH};${PATH_DIR}")
        if(EXISTS ${ZLIB_IMPORTED_LOCATION})
            file(COPY ${ZLIB_IMPORTED_LOCATION} DESTINATION ${QT_BIN_DIR})
        endif()
    elseif(OSX_FRAMEWORK)
        get_filename_component(PATH_DIR ${ZLIB_IMPORTED_LOCATION} PATH)
        get_filename_component(PATH_DIR ${PATH_DIR} PATH)
        get_filename_component(PATH_DIR ${PATH_DIR} PATH)
        if(EXISTS ${PATH_DIR})
            file(COPY ${PATH_DIR} DESTINATION ${QT_BIN_DIR}/Frameworks)
        endif()
    endif()
endif()

add_subdirectory(src)
if(WITH_BINDINGS)
    add_subdirectory(python)
endif()

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# Export package ===============================================================

if(REGISTER_PACKAGE)
    # Export the package for use from the build-tree
    # (this registers the build-tree with a global CMake-registry)
    export(PACKAGE ${PACKAGE_UPPER_NAME})
endif()

# Archiving ====================================================================

set(CPACK_PACKAGE_NAME "${PACKAGE_NAME}")
set(CPACK_PACKAGE_VENDOR "${PACKAGE_VENDOR}")
set(CPACK_PACKAGE_VERSION "${VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PACKAGE_NAME} Installation")
set(CPACK_PACKAGE_RELOCATABLE TRUE)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_GENERATOR "ZIP")
set(CPACK_MONOLITHIC_INSTALL ON)
set(CPACK_STRIP_FILES TRUE)

# Get cpack zip archive name
get_cpack_filename(${VERSION} PROJECT_CPACK_FILENAME)
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_CPACK_FILENAME})

include(CPack)
