/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Framework library
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2012-2018 NextGIS, info@nextgis.ru
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
#ifndef NGFRAMEWORK_MINISPLITTER_H
#define NGFRAMEWORK_MINISPLITTER_H

#include "framework/framework.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))

#include <QSplitter>

class NGFRAMEWORK_EXPORT NGMiniSplitter : public QSplitter
{
public:
    NGMiniSplitter(QWidget *parent = nullptr);
    NGMiniSplitter(Qt::Orientation orientation);
    virtual ~NGMiniSplitter() = default;

protected:
    void init();
};

#endif // QT_VERSION >= 0x050000

#endif // NGFRAMEWORK_MINISPLITTER_H
