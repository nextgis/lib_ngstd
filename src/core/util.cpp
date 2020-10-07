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

#include "core/util.h"


QMap<QString, QVariant> toMap(const CPLJSONObject &root) {
    QMap<QString, QVariant> out;
    for(const CPLJSONObject &child : root.GetChildren()) {
        QString name = QString::fromStdString(child.GetName());
        switch(child.GetType()) {
        case CPLJSONObject::Type::Boolean:
            out[name] = child.ToBool();
            break;
        case CPLJSONObject::Type::String:
            out[name] = QString::fromUtf8(child.ToString().c_str());
            break;
        case CPLJSONObject::Type::Integer:
            out[name] = child.ToInteger();
            break;
        case CPLJSONObject::Type::Long:
            out[name] = child.ToLong();
            break;
        case CPLJSONObject::Type::Double:
            out[name] = child.ToDouble();
            break;
        default:
            out[name] = QString::fromUtf8(child.ToString().c_str());
        }
    }
    return out;
}
