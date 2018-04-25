/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Framework library
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2012-2016 NextGIS, info@nextgis.ru
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 2 of the License, or
*   (at your option) any later version.
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#ifndef NGSTD_FRAMEWORK_H
#define NGSTD_FRAMEWORK_H

#include <QtCore/QtGlobal>

#if defined(NGSTD_FRAMEWORK_LIBRARY)
#  define NGFRAMEWORK_EXPORT Q_DECL_EXPORT
#else
#  if defined(NGSTD_STATIC)
#    define NGFRAMEWORK_EXPORT
#  else
#    define NGFRAMEWORK_EXPORT Q_DECL_IMPORT
#  endif // if defined(NGSTD_FRAMEWORK_STATIC)
#endif // if defined(NGSTD_FRAMEWORK_LIBRARY)

#endif // NGSTD_FRAMEWORK_H
