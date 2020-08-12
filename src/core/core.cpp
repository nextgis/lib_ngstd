/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Core library
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
#include "core/core.h"

#include "core/version.h"
#include "core/util.h"

#include "cpl_json.h"
#include "cpl_string.h"

const char* getVersion()
{
    return NGLIB_VERSION_STRING;
}

QMap<QString, QVariant> memJsonToMap(const QString &str) {
    CPLJSONDocument in;
    if(in.LoadMemory(str.toStdString())) {
        return toMap(in.GetRoot());
    }
    return QMap<QString, QVariant>();
}

QMap<QString, QVariant> jsonToMap(const QString &path)
{
    CPLJSONDocument in;
    if(in.Load(path.toStdString())) {
        return toMap(in.GetRoot());
    }
    return QMap<QString, QVariant>();
}
