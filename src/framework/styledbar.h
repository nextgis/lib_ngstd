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
#ifndef NGFRAMEWORK_STYLEDBAR_H
#define NGFRAMEWORK_STYLEDBAR_H


#include "framework/framework.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))

#include <QWidget>

/**
 * @brief The NGStyledBar class
 */
class NGFRAMEWORK_EXPORT NGStyledBar : public QWidget
{
    Q_OBJECT
public:
    explicit NGStyledBar(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event);

signals:

public slots:
};

/**
 * @brief The NGStyledSeparator class
 */
class Q_DECL_HIDDEN NGStyledSeparator : public QWidget
{
    Q_OBJECT
public:
    NGStyledSeparator(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event);
};

#endif // QT_VERSION >= 0x050000

#endif // NGFRAMEWORK_STYLEDBAR_H
