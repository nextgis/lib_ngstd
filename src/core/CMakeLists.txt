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

project(ngstd_core)

find_anyproject(GDAL REQUIRED)

set(PUBLIC_HEADERS
    ${PROJECT_SOURCE_DIR}/core.h
    ${PROJECT_SOURCE_DIR}/version.h
    ${PROJECT_SOURCE_DIR}/request.h
)

set(PRIVATE_HEADERS
)

set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/core.cpp
    ${PROJECT_SOURCE_DIR}/request.cpp
)

if(GDAL_VERSION VERSION_LESS "2.3")
    find_anyproject(JSONC REQUIRED
        NAMES json-c jsonc json_c
    )
    if(JSONC_FOUND)
        include_directories(${JSONC_INCLUDE_DIRS}/json-c)
    endif()

    set(PRIVATE_HEADERS ${PRIVATE_HEADERS}
        ${PROJECT_SOURCE_DIR}/cpl_json.h
    )
    set(PROJECT_SOURCES ${PROJECT_SOURCES}
        ${PROJECT_SOURCE_DIR}/cpl_json.cpp
    )
endif()

set(TRANSLATIONS
    ${NGSTD_SOURCE_DIR}/translations/${PROJECT_NAME}_ru.ts
    ${NGSTD_SOURCE_DIR}/translations/${PROJECT_NAME}_en.ts
)

if(MSVC)
    add_definitions(-DLIB_NAME=${PROJECT_NAME})
    set(PROJECT_SOURCES ${PROJECT_SOURCES} "${CMAKE_SOURCE_DIR}/res/ngstd.rc")
endif()

string(TOUPPER ${PROJECT_NAME} PROJECT_UPPER_NAME)

macro(ADD_NGSTDCORE_LIB NAME QT_VER)
    set(LIB_NAME ${NAME}${QT_VER})
    set(PACKAGE_NAME ${PROJECT_NAME}${QT_VER})
    string(TOUPPER ${PACKAGE_NAME} PACKAGE_UPPER_NAME)

    if(BUILD_SHARED_LIBS)
        add_library(${LIB_NAME} SHARED ${PUBLIC_HEADERS} ${PRIVATE_HEADERS} ${PROJECT_SOURCES} ${QM_FILES} ${MOC_SRCS${QT_VER}})
        if(MSVC)
            set_target_properties(${LIB_NAME} PROPERTIES IMPORT_SUFFIX "_i.lib")
        endif()
        set_target_properties(${LIB_NAME} PROPERTIES
            DEFINE_SYMBOL ${PROJECT_UPPER_NAME}_LIBRARY
            VERSION ${VERSION}
            SOVERSION ${NGLIB_MAJOR_VERSION}
            CXX_VISIBILITY_PRESET hidden
        )
    elseif(OSX_FRAMEWORK)
        set(INSTALL_CMAKECONF_DIR ${INSTALL_LIB_DIR}/${LIB_NAME}.framework/Resources/CMake CACHE INTERNAL "Installation directory for cmake config files" FORCE)
        set(INSTALL_INC_DIR "${INSTALL_LIB_DIR}/${LIB_NAME}.framework/Headers" CACHE INTERNAL "Installation directory for headers" FORCE)

        add_library(${LIB_NAME} SHARED ${PUBLIC_HEADERS} ${PRIVATE_HEADERS} ${PROJECT_SOURCES} ${QM_FILES} ${MOC_SRCS${QT_VER}})
        set_property(SOURCE ${QM_FILES} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/translations")
        set_property(SOURCE ${PUBLIC_HEADERS} PROPERTY MACOSX_PACKAGE_LOCATION "Headers/core")
        set_target_properties(${LIB_NAME} PROPERTIES
            DEFINE_SYMBOL ${PROJECT_UPPER_NAME}_LIBRARY
            FRAMEWORK TRUE
            FRAMEWORK_VERSION ${FRAMEWORK_VERSION}
            MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${VERSION}
            MACOSX_FRAMEWORK_BUNDLE_VERSION ${VERSION}
            VERSION ${VERSION}
            SOVERSION ${NGLIB_MAJOR_VERSION}
            MACOSX_FRAMEWORK_IDENTIFIER com.nextgis.${PROJECT_NAME}
            XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
        #    PUBLIC_HEADER "${PUBLIC_HEADERS}"
        )
    else()
        add_library(${LIB_NAME} STATIC ${PUBLIC_HEADERS} ${PRIVATE_HEADERS} ${PROJECT_SOURCES} ${QM_FILES} ${MOC_SRCS${QT_VER}})
        add_definitions(-DNGSTD_STATIC)
        if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
            set( CMAKE_CXX_FLAGS "-fpic ${CMAKE_CXX_FLAGS}" )
            set( CMAKE_C_FLAGS   "-fpic ${CMAKE_C_FLAGS}" )
        endif()
    endif()

    if(${QT_VER} EQUAL 5)
        target_link_libraries(${LIB_NAME} PRIVATE Qt5::Core ${GDAL_LIBRARIES})
        if(JSONC_FOUND)
            target_link_libraries(${LIB_NAME} PRIVATE ${JSONC_LIBRARIES})
        endif()
    elseif(${QT_VER} EQUAL 4)
        target_link_libraries(${LIB_NAME} PRIVATE Qt4::QtCore ${GDAL_LIBRARIES})
        if(JSONC_FOUND)
            target_link_libraries(${LIB_NAME} PRIVATE ${JSONC_LIBRARIES})
        endif()
    else()
        message(FATAL_ERROR "Unexpected qt version")
    endif()

    target_include_directories(${LIB_NAME} INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    )

    # Install
    if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
        install(TARGETS ${LIB_NAME}
            EXPORT ${PACKAGE_UPPER_NAME}Targets
            RUNTIME DESTINATION ${INSTALL_BIN_DIR}
            LIBRARY DESTINATION ${INSTALL_LIB_DIR}
            ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
            INCLUDES DESTINATION ${INSTALL_INC_DIR}
            FRAMEWORK DESTINATION ${INSTALL_LIB_DIR}
        )
    endif()

    # Add all targets to the build-tree export set
    export(TARGETS ${LIB_NAME}
        FILE ${PROJECT_BINARY_DIR}/${PACKAGE_UPPER_NAME}Targets.cmake)

    # Create the <Package>Config.cmake file
    configure_file(${CMAKE_SOURCE_DIR}/cmake/PackageConfig.cmake.in
        ${PROJECT_BINARY_DIR}/${PACKAGE_UPPER_NAME}Config.cmake @ONLY)

    if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL)
        # Install the <Package>Config.cmake
        install(FILES
          ${PROJECT_BINARY_DIR}/${PACKAGE_UPPER_NAME}Config.cmake
          DESTINATION ${INSTALL_CMAKECONF_DIR} COMPONENT dev)

        # Install the export set for use with the install-tree
        install(EXPORT ${PACKAGE_UPPER_NAME}Targets DESTINATION ${INSTALL_CMAKECONF_DIR} COMPONENT dev)
    endif()

endmacro()

if(BUILD_QT5)
    add_subdirectory(5)
endif()
if(BUILD_QT4)
    add_subdirectory(4)
endif()
