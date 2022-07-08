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

set(TMP_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/inst")
set(CONFIGURE_ARGS --verbose --build-dir ${CMAKE_CURRENT_BINARY_DIR}/build_tmp --target-dir ${TMP_INSTALL_PREFIX} --no-docstrings)

get_property(QT_QMAKE_EXECUTABLE TARGET Qt5::qmake PROPERTY IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" PATH)
get_filename_component(QT_ROOT_DIR "${QT_BIN_DIR}" PATH)
set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake=${QT_QMAKE_EXECUTABLE})
set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting "QMAKE_RPATHDIR+=${QT_ROOT_DIR}/lib")

find_program(SIP_INSTALL_EXECUTABLE sip-install PATHS ${BASE_PATH}/bin)

get_property(Qt5Core_INCLUDE_DIRS TARGET Qt5::Core PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
foreach(_INC_DIR ${Qt5Core_INCLUDE_DIRS})
    set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${_INC_DIR})
endforeach()
get_property(Qt5Widgets_INCLUDE_DIRS TARGET Qt5::Widgets PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
foreach(_INC_DIR ${Qt5Widgets_INCLUDE_DIRS})
    set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${_INC_DIR})
endforeach()
get_property(Qt5Gui_INCLUDE_DIRS TARGET Qt5::Gui PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
foreach(_INC_DIR ${Qt5Gui_INCLUDE_DIRS})
    set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${_INC_DIR})
endforeach()
set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${PROJECT_SOURCE_DIR}/src)
set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${PROJECT_SOURCE_DIR}/src/core)
set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${PROJECT_SOURCE_DIR}/src/framework)

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

get_target_property(QT_CORE_LIB Qt5::Core IMPORTED_IMPLIB_${BUILD_TYPE})
set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting "LIBS+=${QT_CORE_LIB}")

get_target_property(QT_WIDGETS_LIB Qt5::Widgets IMPORTED_IMPLIB_${BUILD_TYPE})
set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting "LIBS+=${QT_WIDGETS_LIB}")

get_target_property(QT_GUI_LIB Qt5::Gui IMPORTED_IMPLIB_${BUILD_TYPE})
set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting "LIBS+=${QT_GUI_LIB}")

set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting "LIBS+=${CMAKE_BINARY_DIR}/src/core/${CMAKE_BUILD_TYPE}/ngstd_core_i.lib")
set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting "LIBS+=${CMAKE_BINARY_DIR}/src/framework/${CMAKE_BUILD_TYPE}/ngstd_framework_i.lib")

configure_file(${CMAKE_SOURCE_DIR}/cmake/pyproject.toml.in
    ${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml @ONLY
)

add_custom_target(build_py ALL
    COMMAND ${SIP_INSTALL_EXECUTABLE} ${CONFIGURE_ARGS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${LIB_NAME}
    VERBATIM
)

if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    foreach(PY_MODULE core framework)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PY_MODULE}/__init__.py DESTINATION "${NGSTD_PYTHON_DIR}/${PY_MODULE}")
    endforeach()
    
    install(DIRECTORY ${TMP_INSTALL_PREFIX}/ngstd/ DESTINATION "${NGSTD_PYTHON_DIR}")

    install(FILES __init__.py DESTINATION "${NGSTD_PYTHON_DIR}")
    install(FILES loader/ximport.py DESTINATION "${NGSTD_PYTHON_DIR}")
endif()