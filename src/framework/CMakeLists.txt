#***************************************************************************
# Project:  NextGIS framework library
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

project(ngstd_framework)

# Widgets finds its own dependencies.
find_package(Qt5 REQUIRED COMPONENTS Widgets LinguistTools)

qt5_add_resources(RESOURCES res/framework.qrc)

set(PUBLIC_HEADERS
    framework.h
    application.h
    mainwindow.h
    minisplitter.h
    navigationwidget.h
    navigationpane.h
    styledbar.h
)

set(PROJECT_HEADERS
    style.h
)

set(PROJECT_SOURCES
    application.cpp
    mainwindow.cpp
    style.cpp
    minisplitter.cpp
    navigationwidget.cpp
    navigationpane.cpp
    styledbar.cpp
)

#qt5_wrap_ui(UIS_HDRS
#    res/ui/projpropwidget.ui
#)

set(TRANSLATIONS
    ${NGSTD_SOURCE_DIR}/translations/${PROJECT_NAME}_ru.ts
    ${NGSTD_SOURCE_DIR}/translations/${PROJECT_NAME}_en.ts
)

unset(QM_FILES)
foreach(TRANSLATION ${TRANSLATIONS})
    if(EXISTS ${TRANSLATION})
        qt5_add_translation(QM_FILES ${TRANSLATION})
    endif()
endforeach()

if(MSVC)
    add_definitions(-DLIB_NAME=${PROJECT_NAME})
    set(PROJECT_SOURCES ${PROJECT_SOURCES} "${PROJECT_SOURCE_DIR}/res/ngstd.rc")
endif()

string(TOUPPER ${PROJECT_NAME} PROJECT_UPPER_NAME)

if(BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} SHARED ${PUBLIC_HEADERS} ${PROJECT_HEADERS} ${PROJECT_SOURCES} ${RESOURCES})
    if(MSVC)
        set_target_properties(${PROJECT_NAME} PROPERTIES IMPORT_SUFFIX "_i.lib")
    endif()
    set_target_properties(${PROJECT_NAME} PROPERTIES
        DEFINE_SYMBOL ${PROJECT_UPPER_NAME}_LIBRARY
        VERSION ${VERSION}
        SOVERSION ${NGLIB_MAJOR_VERSION}
    )
elseif(OSX_FRAMEWORK)
    add_library(${PROJECT_NAME} SHARED ${PUBLIC_HEADERS} ${PROJECT_HEADERS} ${PROJECT_SOURCES} ${QM_FILES} ${RESOURCES})
    set_property(SOURCE ${QM_FILES} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/translations")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        DEFINE_SYMBOL ${PROJECT_UPPER_NAME}_LIBRARY
        FRAMEWORK TRUE
        FRAMEWORK_VERSION ${FRAMEWORK_VERSION}
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${VERSION}
        MACOSX_FRAMEWORK_BUNDLE_VERSION ${VERSION}
        VERSION ${VERSION}
        SOVERSION ${NGLIB_MAJOR_VERSION}
        MACOSX_FRAMEWORK_IDENTIFIER com.nextgis.${PROJECT_NAME}
        XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
        PUBLIC_HEADER "${PUBLIC_HEADERS}")
else()
    add_library(${PROJECT_NAME} STATIC ${PUBLIC_HEADERS} ${PROJECT_HEADERS} ${PROJECT_SOURCES} ${RESOURCES})
    add_definitions(-DNGSTD_STATIC)
    if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        set( CMAKE_CXX_FLAGS "-fpic ${CMAKE_CXX_FLAGS}" )
        set( CMAKE_C_FLAGS   "-fpic ${CMAKE_C_FLAGS}" )
    endif()
endif()

target_link_libraries(${PROJECT_NAME} Qt5::Widgets ngstd_core)

# Install
if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    install(TARGETS ${PROJECT_NAME}
        #EXPORT targets
        RUNTIME DESTINATION ${INSTALL_BIN_DIR} # at least for dlls
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        FRAMEWORK DESTINATION ${INSTALL_LIB_DIR})
endif()

if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL)
    install(FILES ${PUBLIC_HEADERS} DESTINATION ${INSTALL_INC_DIR})
endif()

if(NOT SKIP_INSTALL_FILES AND NOT SKIP_INSTALL_ALL)
    install(FILES ${QM_FILES} DESTINATION ${INSTALL_TRANSLATIONS_DIR})
endif()
