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
#include "styledbar.h"

#include <QPainter>
#include <QStyleOptionToolBar>

////////////////////////////////////////////////////////////////////////////////
// NGStyledSeparator
////////////////////////////////////////////////////////////////////////////////

NGStyledSeparator::NGStyledSeparator(QWidget *parent) : QWidget(parent)
{
    setFixedWidth(4);
}

void NGStyledSeparator::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    QStyleOption option;
    option.rect = rect();
    option.state = QStyle::State_Horizontal;
    option.palette = palette();
    style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &option,
                           &painter, this);
}

////////////////////////////////////////////////////////////////////////////////
// NGStyledBar
////////////////////////////////////////////////////////////////////////////////

NGStyledBar::NGStyledBar(QWidget *parent) : QWidget(parent)
{
    setProperty("panelwidget", true);
    setProperty("panelwidget_singlerow", true);
}

void NGStyledBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    QStyleOptionToolBar option;
    option.rect = rect();
    option.state = QStyle::State_Horizontal;
    style()->drawControl(QStyle::CE_ToolBar, &option, &painter, this);
}
