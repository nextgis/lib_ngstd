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

find_anyproject(Qt4 REQUIRED COMPONENTS QtCore QtNetwork QtSvg)
if(BUILD_SHARED_LIBS OR OSX_FRAMEWORK)
    if(WITH_ZLIB_EXTERNAL)
        get_target_property(ZLIB_IMPORTED_LOCATION ${ZLIB_LIBRARIES} IMPORTED_LOCATION_RELEASE)
        get_target_property(RCC_PATH Qt4::lupdate IMPORTED_LOCATION)
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

foreach(TRANSLATION ${TRANSLATIONS})
    if(EXISTS ${TRANSLATION})
        qt4_add_translation(QM_FILES ${TRANSLATION})
    endif()
endforeach()

qt4_wrap_cpp(MOC_SRCS4 ${PUBLIC_HEADERS})
add_ngstdcore_lib(${PROJECT_NAME} 4)

if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL)
    install(FILES ${PUBLIC_HEADERS} DESTINATION ${INSTALL_INC_DIR}/core)
endif()

if(NOT SKIP_INSTALL_FILES AND NOT SKIP_INSTALL_ALL)
    install(FILES ${QM_FILES} DESTINATION ${INSTALL_TRANSLATIONS_DIR})
endif()
