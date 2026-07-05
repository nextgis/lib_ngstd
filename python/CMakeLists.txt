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

find_package(SIP REQUIRED)
if(WITH_QT6)
    find_anyproject(PyQt6)
    if(NOT PyQt6_FOUND)
        message(WARNING "PyQt6 not found — falling back to PyQt5 and disabling WITH_QT6")
        set(WITH_QT6 OFF CACHE BOOL "" FORCE)
        set(QT_VERSION_BASE Qt5 CACHE INTERNAL "")
        find_anyproject(PyQt5 REQUIRED)
    endif()
else()
    find_anyproject(PyQt5 REQUIRED)
endif()

if (SIP_INSTALL_EXECUTABLE)
    set(TMP_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/inst")
    set(CONFIGURE_ARGS --verbose --build-dir ${CMAKE_CURRENT_BINARY_DIR}/build_tmp --target-dir ${TMP_INSTALL_PREFIX} --no-docstrings)

    if(TARGET ${QT_VERSION_BASE}::qmake)
        get_property(QT_QMAKE_EXECUTABLE TARGET ${QT_VERSION_BASE}::qmake PROPERTY IMPORTED_LOCATION)
        get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" PATH)
        get_filename_component(QT_ROOT_DIR "${QT_BIN_DIR}" PATH)
        set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake=${QT_QMAKE_EXECUTABLE})
        set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting "QMAKE_RPATHDIR+=${QT_ROOT_DIR}/lib")

        if(TARGET ${QT_VERSION_BASE}::Core)
            get_property(QtCore_INCLUDE_DIRS TARGET ${QT_VERSION_BASE}::Core PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
            foreach(_INC_DIR ${QtCore_INCLUDE_DIRS})
                set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${_INC_DIR})
            endforeach()
        endif()
        if(TARGET ${QT_VERSION_BASE}::Widgets)
            get_property(QtWidgets_INCLUDE_DIRS TARGET ${QT_VERSION_BASE}::Widgets PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
            foreach(_INC_DIR ${QtWidgets_INCLUDE_DIRS})
                set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${_INC_DIR})
            endforeach()
        endif()
        if(TARGET ${QT_VERSION_BASE}::Gui)
            get_property(QtGui_INCLUDE_DIRS TARGET ${QT_VERSION_BASE}::Gui PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
            foreach(_INC_DIR ${QtGui_INCLUDE_DIRS})
                set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${_INC_DIR})
            endforeach()
        endif()
    else()
        # Fallback: try to find qmake executable in PATH
        if(WITH_QT6)
            find_program(QT_QMAKE_EXECUTABLE NAMES qmake6 qmake-qt6 qmake)
        else()
            find_program(QT_QMAKE_EXECUTABLE NAMES qmake-qt5 qmake5 qmake)
        endif()
        if(NOT QT_QMAKE_EXECUTABLE)
            message(FATAL_ERROR "Could not find qmake executable for ${QT_VERSION_BASE}. Install Qt development tools or set QT_QMAKE_EXECUTABLE.")
        endif()
        get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" PATH)
        get_filename_component(QT_ROOT_DIR "${QT_BIN_DIR}" PATH)
        set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake=${QT_QMAKE_EXECUTABLE})
        set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting "QMAKE_RPATHDIR+=${QT_ROOT_DIR}/lib")
        # Try to query qmake for Qt include dirs and add them to sip's INCLUDEPATH
        execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_HEADERS
                        OUTPUT_VARIABLE QT_QMAKE_HEADERS
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                        ERROR_QUIET)
        if(QT_QMAKE_HEADERS)
            set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${QT_QMAKE_HEADERS})
            set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${QT_QMAKE_HEADERS}/QtWidgets)
            set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${QT_QMAKE_HEADERS}/QtGui)
            set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${QT_QMAKE_HEADERS}/QtCore)
        endif()
        # Note: include directories from Qt targets are unavailable; rely on qmake
    endif()
    set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${PROJECT_SOURCE_DIR}/src)
    set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${PROJECT_SOURCE_DIR}/src/core)
    set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${PROJECT_SOURCE_DIR}/src/framework)

    if(WIN32)
        string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

        get_target_property(QT_CORE_LIB Qt5::Core IMPORTED_IMPLIB_${BUILD_TYPE})
        set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting "LIBS+=${QT_CORE_LIB}")

        get_target_property(QT_WIDGETS_LIB Qt5::Widgets IMPORTED_IMPLIB_${BUILD_TYPE})
        set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting "LIBS+=${QT_WIDGETS_LIB}")

        if(TARGET ${QT_VERSION_BASE}::qmake)
            get_property(QT_QMAKE_EXECUTABLE TARGET ${QT_VERSION_BASE}::qmake PROPERTY IMPORTED_LOCATION)
            get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" PATH)
            get_filename_component(QT_ROOT_DIR "${QT_BIN_DIR}" PATH)
            set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake=${QT_QMAKE_EXECUTABLE})
            set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting "QMAKE_RPATHDIR+=${QT_ROOT_DIR}/lib")

            if(TARGET ${QT_VERSION_BASE}::Core)
                get_property(QtCore_INCLUDE_DIRS TARGET ${QT_VERSION_BASE}::Core PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
                foreach(_INC_DIR ${QtCore_INCLUDE_DIRS})
                    set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${_INC_DIR})
                endforeach()
            endif()
            if(TARGET ${QT_VERSION_BASE}::Widgets)
                get_property(QtWidgets_INCLUDE_DIRS TARGET ${QT_VERSION_BASE}::Widgets PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
                foreach(_INC_DIR ${QtWidgets_INCLUDE_DIRS})
                    set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${_INC_DIR})
                endforeach()
            endif()
            if(TARGET ${QT_VERSION_BASE}::Gui)
                get_property(QtGui_INCLUDE_DIRS TARGET ${QT_VERSION_BASE}::Gui PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
                foreach(_INC_DIR ${QtGui_INCLUDE_DIRS})
                    set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting INCLUDEPATH+=${_INC_DIR})
                endforeach()
            endif()
        else()
            # Fallback: try to find qmake executable in PATH (prefer matching Qt major)
            if(WITH_QT6)
                find_program(QT_QMAKE_EXECUTABLE NAMES qmake6 qmake-qt6 qmake)
            else()
                find_program(QT_QMAKE_EXECUTABLE NAMES qmake-qt5 qmake5 qmake)
            endif()
            if(NOT QT_QMAKE_EXECUTABLE)
                message(FATAL_ERROR "Could not find qmake executable for ${QT_VERSION_BASE}. Install Qt development tools or set QT_QMAKE_EXECUTABLE.")
            endif()
            get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" PATH)
            get_filename_component(QT_ROOT_DIR "${QT_BIN_DIR}" PATH)
            set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake=${QT_QMAKE_EXECUTABLE})
            set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting "QMAKE_RPATHDIR+=${QT_ROOT_DIR}/lib")
            # Note: include directories from Qt targets are unavailable; rely on qmake
        endif()
        if(TARGET ${QT_VERSION_BASE}::Core)
            get_target_property(QT_CORE_LIB ${QT_VERSION_BASE}::Core IMPORTED_IMPLIB_${BUILD_TYPE})
            set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting "LIBS+=${QT_CORE_LIB}")
        endif()
        if(TARGET ${QT_VERSION_BASE}::Widgets)
            get_target_property(QT_WIDGETS_LIB ${QT_VERSION_BASE}::Widgets IMPORTED_IMPLIB_${BUILD_TYPE})
            set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting "LIBS+=${QT_WIDGETS_LIB}")
        endif()
        if(TARGET ${QT_VERSION_BASE}::Gui)
            get_target_property(QT_GUI_LIB ${QT_VERSION_BASE}::Gui IMPORTED_IMPLIB_${BUILD_TYPE})
            set(CONFIGURE_ARGS ${CONFIGURE_ARGS} --qmake-setting "LIBS+=${QT_GUI_LIB}")
        endif()

    endif()

    configure_file(${CMAKE_SOURCE_DIR}/cmake/pyproject.toml.in
        ${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml @ONLY)

    add_custom_target(build_py ALL
        COMMAND ${SIP_INSTALL_EXECUTABLE} ${CONFIGURE_ARGS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ngstd_core ngstd_framework
        VERBATIM
    )

    if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
        install(DIRECTORY ${TMP_INSTALL_PREFIX}/ngstd/ DESTINATION "${NGSTD_PYTHON_DIR}")
    endif()
else(SIP_INSTALL_EXECUTABLE)
    include(SIPMacros)

    file(GLOB_RECURSE sip_files_core sip/core/*.sip)
    GENERATE_SIP_PYTHON_MODULE_CODE(ngstd._core core/core.sip "${sip_files_core}" cpp_files)
    BUILD_SIP_PYTHON_MODULE(ngstd._core core/core.sip ${cpp_files} "")

    file(GLOB_RECURSE sip_files_framework sip/framework/*.sip)
    GENERATE_SIP_PYTHON_MODULE_CODE(ngstd._framework framework/framework.sip "${sip_files_framework}" cpp_files)
    BUILD_SIP_PYTHON_MODULE(ngstd._framework framework/framework.sip ${cpp_files} "")
endif(SIP_INSTALL_EXECUTABLE)

if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    foreach(PY_MODULE core framework)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PY_MODULE}/__init__.py DESTINATION "${NGSTD_PYTHON_DIR}/${PY_MODULE}")
    endforeach()

    install(FILES __init__.py DESTINATION "${NGSTD_PYTHON_DIR}")
    install(FILES loader/ximport.py DESTINATION "${NGSTD_PYTHON_DIR}")
endif()