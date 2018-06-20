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

if(WITH_ZLIB_EXTERNAL)
    # ZLIB needed for uic, rcc, lrelease and lupdate
    find_anyproject(ZLIB REQUIRED)
endif()

configure_file(${CMAKE_SOURCE_DIR}/cmake/PackageConfigFullLocal.cmake.in
    ${PROJECT_BINARY_DIR}/${PACKAGE_UPPER_NAME}Config.cmake @ONLY
)

# Create the <Package>Config.cmake file
configure_file(${CMAKE_SOURCE_DIR}/cmake/PackageConfigFull.cmake.in
    ${PROJECT_BINARY_DIR}/src/${PACKAGE_UPPER_NAME}Config.cmake @ONLY
)

if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL)
    # Install the <Package>Config.cmake
    install(FILES
      ${PROJECT_BINARY_DIR}/src/${PACKAGE_UPPER_NAME}Config.cmake
      DESTINATION ${INSTALL_CMAKECONF_DIR} COMPONENT dev)
endif()

set(NGLIBS
    core
    framework
)

foreach(NGLIB ${NGLIBS})
    add_subdirectory(${NGLIB})
endforeach()
