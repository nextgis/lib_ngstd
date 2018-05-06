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
#ifndef NGFRAMEWORK_SIGNINBUTTON_H
#define NGFRAMEWORK_SIGNINBUTTON_H

#include "framework/framework.h"

#include <QDialog>
#include <QToolButton>

class NGFRAMEWORK_EXPORT NGSignInButton : public QToolButton
{
    Q_OBJECT
public:
    NGSignInButton(const QString &clientId, const QString &scope = "user_info.read",
                   QWidget * parent = nullptr);

signals:
    void userInfoUpdated();
    void supportInfoUpdated();

public slots:
    void onClick();
    void onUserInfoUpdated();

private:
    QDialog *m_signDialog;
};

#endif // NGFRAMEWORK_SIGNINBUTTON_H
