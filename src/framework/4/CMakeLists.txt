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

foreach(TRANSLATION ${TRANSLATIONS})
    if(EXISTS ${TRANSLATION})
        qt4_add_translation(QM_FILES ${TRANSLATION})
    endif()
endforeach()


set(ALL_HEADERS ${PRIVATE_HEADERS} ${PUBLIC_HEADERS} ${ACCESS_HHEADERS})
set(PROJECT_SOURCES ${PROJECT_SOURCES} ${ACCESS_CSOURCES})

qt4_add_resources(RESOURCES_QT4 ${QRC_FILES})
qt4_wrap_ui(UIS_HDRS_QT4 ${ACCESS_UIS})
qt4_wrap_cpp(MOC_SRCS4 ${ALL_HEADERS})

add_ngstdfrw_lib(${PROJECT_NAME} 4)

if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL)
    install(FILES ${PUBLIC_HEADERS} DESTINATION ${INSTALL_INC_DIR}/framework)
    install(FILES ${ACCESS_HHEADERS} DESTINATION ${INSTALL_INC_DIR}/framework/access)
endif()

if(NOT SKIP_INSTALL_FILES AND NOT SKIP_INSTALL_ALL)
    install(FILES ${QM_FILES} DESTINATION ${INSTALL_TRANSLATIONS_DIR})
endif()
