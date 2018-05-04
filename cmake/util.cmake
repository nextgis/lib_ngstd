################################################################################
# Project:  NextGIS GIS libraries
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov, polimax@mail.ru
################################################################################
# Copyright (C) 2015-2018, NextGIS <info@nextgis.com>
# Copyright (C) 2015-2018 Dmitry Baryshnikov
#
# This script is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This script is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this script.  If not, see <http://www.gnu.org/licenses/>.
################################################################################

function(check_version major minor patch)

    # parse the version number from gdal_version.h and include in
    # major, minor and rev parameters

    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/src/core/version.h VERSION_H_CONTENTS)

    string(REGEX MATCH "NGLIB_MAJOR_VERSION[ \t]+([0-9]+)"
      MAJOR_VERSION ${VERSION_H_CONTENTS})
    string (REGEX MATCH "([0-9]+)"
      MAJOR_VERSION ${MAJOR_VERSION})
    string(REGEX MATCH "NGLIB_MINOR_VERSION[ \t]+([0-9]+)"
      MINOR_VERSION ${VERSION_H_CONTENTS})
    string (REGEX MATCH "([0-9]+)"
      MINOR_VERSION ${MINOR_VERSION})
    string(REGEX MATCH "NGLIB_PATCH_NUMBER[ \t]+([0-9]+)"
        PATCH_NUMBER ${VERSION_H_CONTENTS})
    string (REGEX MATCH "([0-9]+)"
        PATCH_NUMBER ${PATCH_NUMBER})

    set(${major} ${MAJOR_VERSION} PARENT_SCOPE)
    set(${minor} ${MINOR_VERSION} PARENT_SCOPE)
    set(${patch} ${PATCH_NUMBER} PARENT_SCOPE)

endfunction(check_version)


function(report_version name ver)

    string(ASCII 27 Esc)
    set(BoldYellow  "${Esc}[1;33m")
    set(ColourReset "${Esc}[m")

    message("${BoldYellow}${name} version ${ver}${ColourReset}")

endfunction()

function(status_message text)

    string(ASCII 27 Esc)
    set(BoldGreen   "${Esc}[1;32m")
    set(ColourReset "${Esc}[m")

    message(STATUS "${BoldGreen}${text}${ColourReset}")

endfunction()

function(warning_message text)

    string(ASCII 27 Esc)
    set(BoldBlue   "${Esc}[1;34m")
    set(ColourReset "${Esc}[m")

    message(STATUS "${BoldGreen}${text}${ColourReset}")

endfunction()

# macro to find packages on the host OS
macro( find_exthost_package )
if(CMAKE_CROSSCOMPILING)
    set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
    set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER )
    set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER )

    find_package( ${ARGN} )

    set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY )
    set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
    set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
else()
    find_package( ${ARGN} )
endif()
endmacro()


# macro to find programs on the host OS
macro( find_exthost_program )
if(CMAKE_CROSSCOMPILING)
    set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
    set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER )
    set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER )

    find_program( ${ARGN} )

    set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY )
    set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
    set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
else()
    find_program( ${ARGN} )
endif()
endmacro()

# macro to find path on the host OS
macro( find_exthost_path )
if(CMAKE_CROSSCOMPILING)
    set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
    set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER )
    set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER )

    find_path( ${ARGN} )

    set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY )
    set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
    set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
else()
    find_path( ${ARGN} )
endif()
endmacro()

function(get_cpack_filename ver name)
get_compiler_version(COMPILER)
if(BUILD_STATIC_LIBS)
    set(STATIC_PREFIX "static-")
endif()

set(${name} ${PROJECT_NAME}-${STATIC_PREFIX}${ver}-${COMPILER} PARENT_SCOPE)
endfunction()

function(get_compiler_version ver)
## Limit compiler version to 2 or 1 digits
string(REPLACE "." ";" VERSION_LIST ${CMAKE_C_COMPILER_VERSION})
list(LENGTH VERSION_LIST VERSION_LIST_LEN)
if(VERSION_LIST_LEN GREATER 2 OR VERSION_LIST_LEN EQUAL 2)
    list(GET VERSION_LIST 0 COMPILER_VERSION_MAJOR)
    #list(GET VERSION_LIST 1 COMPILER_VERSION_MINOR)
    set(COMPILER_VERSION_MINOR 0)
    set(COMPILER ${CMAKE_C_COMPILER_ID}-${COMPILER_VERSION_MAJOR}.${COMPILER_VERSION_MINOR})
else()
    set(COMPILER ${CMAKE_C_COMPILER_ID}-${CMAKE_C_COMPILER_VERSION})
endif()

if(WIN32)
    if(CMAKE_CL_64)
        set(COMPILER "${COMPILER}-64bit")
    endif()
endif()

set(${ver} ${COMPILER} PARENT_SCOPE)
endfunction()
