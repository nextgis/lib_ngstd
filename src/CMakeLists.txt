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

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include(FindAnyProject)

if(WITH_ZLIB_EXTERNAL)
    # ZLIB needed for uic, rcc, lrelease and lupdate
    find_anyproject(ZLIB REQUIRED)
endif()

if(BUILD_QT5)
    # Widgets finds its own dependencies.
    find_anyproject(Qt5 REQUIRED COMPONENTS Core Widgets Network LinguistTools)

    if(WITH_ZLIB_EXTERNAL)
        get_target_property(ZLIB_IMPORTED_LOCATION ${ZLIB_LIBRARIES} IMPORTED_LOCATION_RELEASE)
        get_target_property(RCC_PATH Qt5::rcc IMPORTED_LOCATION)
        get_filename_component(QT_BIN_DIR ${RCC_PATH} PATH)
        if(WIN32)
            get_filename_component(PATH_DIR ${ZLIB_IMPORTED_LOCATION} PATH)
            set(ENV{PATH} "$ENV{PATH};${PATH_DIR}")
            file(COPY ${ZLIB_IMPORTED_LOCATION} DESTINATION ${QT_BIN_DIR})
        elseif(OSX_FRAMEWORK)
            get_filename_component(PATH_DIR ${ZLIB_IMPORTED_LOCATION} PATH)
            get_filename_component(PATH_DIR ${PATH_DIR} PATH)
            get_filename_component(PATH_DIR ${PATH_DIR} PATH)
            file(COPY ${PATH_DIR} DESTINATION ${QT_BIN_DIR}/Frameworks)
        endif()
    endif()
endif()

# Create the <Package>Config.cmake file
configure_file(${CMAKE_SOURCE_DIR}/cmake/PackageConfigFull.cmake.in
    ${PROJECT_BINARY_DIR}/${PACKAGE_UPPER_NAME}Config.cmake @ONLY)

if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL)
    # Install the <Package>Config.cmake
    install(FILES
      ${PROJECT_BINARY_DIR}/${PACKAGE_UPPER_NAME}Config.cmake
      DESTINATION ${INSTALL_CMAKECONF_DIR} COMPONENT dev)
endif()

foreach(NGLIB ${NGLIBS})
    add_subdirectory(${NGLIB})
endforeach()
