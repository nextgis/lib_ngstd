/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Core Library
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2012-2020 NextGIS, info@nextgis.ru
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
#ifndef NGSTD_CORE_H
#define NGSTD_CORE_H

#include <QtCore/QtGlobal>

#include <QVariant>

#if defined(NGSTD_CORE_LIBRARY)
#  define NGCORE_EXPORT Q_DECL_EXPORT
#else
#  if defined(NGSTD_STATIC)
#    define NGCORE_EXPORT
#  else
#    define NGCORE_EXPORT Q_DECL_IMPORT
#  endif // if defined(NGCORE_STATIC)
#endif // if defined(NGSTD_CORE_LIBRARY)


NGCORE_EXPORT const char *getVersion();
NGCORE_EXPORT QMap<QString, QVariant> jsonToMap(const QString &path);
NGCORE_EXPORT QMap<QString, QVariant> memJsonToMap(const QString &path);

#endif // NGSTD_CORE_H
