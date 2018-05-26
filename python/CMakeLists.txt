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

if(BUILD_QT4)

    include(FindAnyProject)

    find_anyproject(Qt4 REQUIRED COMPONENTS QtCore QtGui QtNetwork QtSvg)
    find_anyproject(SIP REQUIRED)
    find_anyproject(PyQt4 REQUIRED)

    include(SIPMacros)

    # setup SIP variables
    set(PYQT_SIP_FLAGS ${PYQT4_SIP_FLAGS})
    set(PYQT_SIP_DIR ${PYQT4_SIP_DIR})

    set(SIP_INCLUDES ${PYQT_SIP_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
    set(SIP_CONCAT_PARTS 4)
    separate_arguments(PYQT_SIP_FLAGS)

    include_directories(
        ${NGSTD_SOURCE_DIR}/src
        ${NGSTD_BINARY_DIR}
        ${NGSTD_BINARY_DIR}/src
        ${PYTHON_INCLUDE_PATH}
    )

    set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES}
        QT5_SUPPORT CHIMERA_NULL_VARIANT_CONVERSION ANDROID ARM HAVE_TOUCH
        MOBILITY_LOCATION
    )

    if(PYQT4_VERSION_NUM LESS 263680)	# 0x040600
        set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} PROXY_FACTORY)
    endif()

    if(NOT PYQT4_VERSION_NUM LESS 263941 AND QT4_BUILD)	# 0x040705
        set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} QSETINT_CONVERSION)
    endif()

    if(NOT PYQT4_VERSION_NUM LESS 264194)	# 0x040802
        set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} QSETTYPE_CONVERSION)
    endif()

    if(PYQT4_VERSION_NUM LESS 264196)       # 0x040804
        set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} QLISTCONSTPTR_CONVERSION)
    endif()

    if(NOT PYQT4_VERSION_NUM LESS 264453)	# 0x040905
        set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} QVECTORINT_CONVERSION)
    endif()

    set(PY_MODULES
        core
        framework
    )

    foreach(PY_MODULE ${PY_MODULES})
        file(GLOB_RECURSE sip_files_${PY_MODULE} ${PY_MODULE}/*.sip)
        set(SIP_EXTRA_FILES_DEPEND ${sip_files_${PY_MODULE}})
        set(SIP_EXTRA_OPTIONS ${PYQT_SIP_FLAGS} -o -a ${CMAKE_CURRENT_BINARY_DIR}/ngstd.${PY_MODULE}.api)

        # FIXME: Not support only PyQt4 and BUILD_QT4
        set(MODULES_SIP ${MODULES_SIP} ngstd_${PY_MODULE}4)
        add_sip_python_module(ngstd._${PY_MODULE} ${PY_MODULE}/${PY_MODULE}.sip ${MODULES_SIP} ${TARGET_LINK_LIB})

        add_custom_target(py${PY_MODULE} ALL)
        add_dependencies(py${PY_MODULE} python_module_ngstd__${PY_MODULE})
        file(GLOB_RECURSE PY_FILES "${PY_MODULE}/*.py")
        install(FILES ${PY_FILES} DESTINATION "${NGSTD_PYTHON_DIR}/${PY_MODULE}")
    endforeach()

    install(FILES __init__.py DESTINATION "${NGSTD_PYTHON_DIR}")
endif()
