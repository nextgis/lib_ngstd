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
#ifndef NGGUIAPPLICATION_H
#define NGGUIAPPLICATION_H

#include "core/application.h"
#include "framework/mainwindow.h"

#include <QSplashScreen>

/**
 * @brief The NextGIS GUI Application class
 */
class NGFRAMEWORK_EXPORT NGGUIApplication : public NGCoreApplication
{
public:
    NGGUIApplication(const QString& applicationName, const QString& version);
    virtual ~NGGUIApplication() override;
    virtual void init(int &argc, char **argv) override;
    
public:
    static QString style();
    
protected:
    virtual void createMainWindow();
    virtual QPixmap createSplash(const QColor& bkColor = QColor(0,0,0,0));
    virtual void setStyle(const QString &themeName);
    
private:
    void setSplashBackground(QPainter &painter, const QColor& bkColor = QColor(0,0,0,0));

    // NGCoreApplication interface
protected:
    virtual void createApplication(int &argc, char **argv) override;

protected:
    NGMainWindow* m_wnd;
};

#endif // NGGUIAPPLICATION_H
