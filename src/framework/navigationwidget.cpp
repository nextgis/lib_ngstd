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

#include "navigationwidget.h"
#include "navigationpane.h"

#if QT_VERSION >= 0x050000

#include <QSettings>

////////////////////////////////////////////////////////////////////////////////
// NGNavigationWidget
////////////////////////////////////////////////////////////////////////////////

NGNavigationWidget::NGNavigationWidget(QWidget *parent) : NGMiniSplitter (parent)
{
    setOrientation(Qt::Vertical);
}

NGNavigationWidget::~NGNavigationWidget()
{

}

void NGNavigationWidget::addPane(INGNavigationPane *pane)
{
    if(nullptr == pane)
        return;
    m_panes << pane;
}

NGNavigationPaneHolder *NGNavigationWidget::addPaneHolder()
{
    NGNavigationPaneHolder *holder = new NGNavigationPaneHolder(this);
    addWidget(holder);
    return holder;
}

void NGNavigationWidget::removePaneHolder(NGNavigationPaneHolder *pane)
{
    if(count() == 1) {
        // Hide widget
        hide();
    }
    else {
        pane->hide();
        pane->deleteLater();
    }
}

void NGNavigationWidget::writeSettings()
{
    QSettings settings;
    settings.beginGroup("NavigationWidget");
    QStringList panes;
    for(int i = 0; i < count(); ++i) {
        NGNavigationPaneHolder *holder =
                static_cast<NGNavigationPaneHolder *>(widget(i));

        panes << holder->currentWidgetName();
    }
    settings.setValue("panes", panes);
    settings.setValue("splitter.sizes", saveState());
    settings.endGroup();
}

void NGNavigationWidget::readSettings()
{
    QSettings settings;
    settings.beginGroup("NavigationWidget");
    QStringList panes = settings.value("panes").toStringList();
    if(panes.empty()) {
        if(!m_panes.empty()) {
            // Add default pane
            NGNavigationPaneHolder *holder = addPaneHolder();
            holder->selectPane(m_panes[0]->name());
        }
    }
    else {
        for(const QString &pane : panes) {
            NGNavigationPaneHolder *holder = addPaneHolder();
            holder->selectPane(pane);
        }
    }
    restoreState(settings.value("splitter.sizes").toByteArray());
    settings.endGroup();
}

INGNavigationPane *NGNavigationWidget::paneByName(const QString &name) const
{
    foreach(INGNavigationPane *pane, m_panes) {
        if(pane->name() == name) {
            return pane;
        }
    }
    return nullptr;
}

QList<INGNavigationPane *> NGNavigationWidget::panes() const
{
    return m_panes;
}

#endif // QT_VERSION >= 0x050000
