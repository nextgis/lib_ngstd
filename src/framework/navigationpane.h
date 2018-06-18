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
#ifndef NGFRAMEWORK_NAVIGATIONPANE_H
#define NGFRAMEWORK_NAVIGATIONPANE_H

#include "framework/navigationwidget.h"
#include "framework/styledbar.h"

#include <QComboBox>
#include <QMenu>
#include <QWidget>

class NGFRAMEWORK_EXPORT NGNavigationPaneHolder : public QWidget
{
    Q_OBJECT
public:
    explicit NGNavigationPaneHolder(NGNavigationWidget *parent = nullptr);
    virtual ~NGNavigationPaneHolder() = default;
    void selectPane(const QString &name);
    void selectComboboxItem(const QString &name);

    QString currentWidgetName() const;
    QWidget *currentWidget() const;

signals:

public slots:

protected:
    void populateSplitMenu();
    void onSplit();
    void onClose();
    void comboBoxIndexChanged(const QString &text);

protected:
    QComboBox *m_navigationComboBox;
    QMenu *m_splitMenu;
    QWidget *m_currentWidget;
    QString m_currentWidgetName;
};

#endif // NGFRAMEWORK_NAVIGATIONPANE_H
