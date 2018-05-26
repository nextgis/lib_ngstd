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
#ifndef NGFRAMEWORK_NAVIGATIONWIDGET_H
#define NGFRAMEWORK_NAVIGATIONWIDGET_H

#include "framework/minisplitter.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))

#include <QIcon>

/**
 * @brief The INGNavigationPane interface class
 */
class INGNavigationPane
{
public:
    virtual ~INGNavigationPane() = default;
    virtual QString name() const = 0;
    virtual QIcon icon() const { return QIcon(); } // Return empty icon
    virtual QWidget *widget() const = 0;
};

class NGNavigationPaneHolder;
/**
 * @brief The NGNavigationWidget class
 */
class NGFRAMEWORK_EXPORT NGNavigationWidget : public NGMiniSplitter
{
    Q_OBJECT
public:
    NGNavigationWidget(QWidget *parent = nullptr);
    virtual ~NGNavigationWidget();
    virtual void addPane(INGNavigationPane *pane);
    virtual NGNavigationPaneHolder *addPaneHolder();
    virtual void removePaneHolder(NGNavigationPaneHolder *pane);
    virtual void writeSettings();
    virtual void readSettings();

    INGNavigationPane *paneByName(const QString &name) const;
    QList<INGNavigationPane *> panes() const;

protected:
    QList<INGNavigationPane *> m_panes;
};

#endif // QT_VERSION >= 0x050000

#endif // NGFRAMEWORK_NAVIGATIONWIDGET_H
