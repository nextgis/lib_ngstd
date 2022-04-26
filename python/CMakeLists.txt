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

find_anyproject(PyQt5 REQUIRED)

set(SIP_INCLUDE_DIRS ${PYQT5_PYTHON_DIR}/PyQt5/bindings)
set(NGSTD_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/build)

get_filename_component(NGSTD_TARGET_DIR ${CMAKE_BINARY_DIR}/${NGSTD_PYTHON_DIR} PATH)

get_property(Qt5Core_INCLUDE_DIRS TARGET Qt5::Core PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
foreach(_INC_DIR ${Qt5Core_INCLUDE_DIRS})
    set(INCLUDE_DIRS ${INCLUDE_DIRS} "\"${_INC_DIR}\",")
endforeach()
get_property(Qt5Widgets_INCLUDE_DIRS TARGET Qt5::Widgets PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
foreach(_INC_DIR ${Qt5Widgets_INCLUDE_DIRS})
    set(INCLUDE_DIRS ${INCLUDE_DIRS} "\"${_INC_DIR}\",")
endforeach()
get_property(Qt5Gui_INCLUDE_DIRS TARGET Qt5::Gui PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
foreach(_INC_DIR ${Qt5Gui_INCLUDE_DIRS})
    set(INCLUDE_DIRS ${INCLUDE_DIRS} "\"${_INC_DIR}\",")
endforeach()

set(INCLUDE_DIRS ${INCLUDE_DIRS} "\"${CMAKE_SOURCE_DIR}/src\",\"${CMAKE_SOURCE_DIR}/src/core\"")

string(REPLACE ";" "," INCLUDE_DIRS ${INCLUDE_DIRS})

get_property(QT_QMAKE_EXECUTABLE TARGET Qt5::qmake PROPERTY IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" PATH)
get_filename_component(QT_ROOT_DIR "${QT_BIN_DIR}" PATH)

set(LIBRARY_DIRS "\"${QT_ROOT_DIR}/lib\",\"${CMAKE_BINARY_DIR}/src/core/${CMAKE_BUILD_TYPE}\",\"${CMAKE_BINARY_DIR}/src/framework/${CMAKE_BUILD_TYPE}\"")

configure_file(${CMAKE_SOURCE_DIR}/cmake/pyproject.toml.in
    ${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml @ONLY
)

# Find sip-install
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -m site --user-base
    RESULT_VARIABLE _process_status
    OUTPUT_VARIABLE _process_output
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(${_process_status} EQUAL 0)
    string(STRIP ${_process_output} BASE_PATH)
endif()

find_program(SIP_INSTALL_EXECUTABLE sip-install PATHS ${BASE_PATH}/bin)

add_custom_target(build_py ALL
    COMMAND ${SIP_INSTALL_EXECUTABLE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ngstd_core ngstd_framework
    VERBATIM
)

if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    foreach(PY_MODULE "core;framework")
        file(GLOB_RECURSE PY_FILES "${PY_MODULE}/*.py")
        install(FILES ${PY_FILES} DESTINATION "${NGSTD_PYTHON_DIR}/${PY_MODULE}")
    endforeach()

    install(FILES __init__.py DESTINATION "${NGSTD_PYTHON_DIR}")
    install(FILES loader/ximport.py DESTINATION "${NGSTD_PYTHON_DIR}")
endif()