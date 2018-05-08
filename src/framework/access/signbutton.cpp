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

#include "access.h"
#include "signbutton.h"
#include "signdialog.h"

#include <QIcon>

NGSignInButton::NGSignInButton(const QString &clientId, const QString &scope,
                               QWidget * parent) :
    QToolButton (parent)
{
    NGAccess::instance().setScope(scope);
    NGAccess::instance().setClientId(clientId);

    if(NGAccess::instance().isUserAuthorized()) {
        setIcon(NGAccess::instance().avatar());
    }
    else {
        setIcon(QIcon(":/icons/person-blue.svg"));
    }

    connect(this, SIGNAL(clicked()), this, SLOT(onClick()));
    connect(&NGAccess::instance(), SIGNAL(userInfoUpdated()), this, SLOT(onUserInfoUpdated()));
    connect(&NGAccess::instance(), SIGNAL(supportInfoUpdated()), this, SIGNAL(supportInfoUpdated()));

    m_signDialog = new NGSignDialog(this);
}

void NGSignInButton::onClick()
{
    NGSignDialog *dlg = qobject_cast<NGSignDialog*>(m_signDialog);
    QSize dlgSize = dlg->size();
    QPoint pos1 = mapToGlobal(pos());
    QSize btnSize = size();
    QPoint pos2(pos1.x() - dlgSize.width() - btnSize.width() / 2,
                pos1.y() + btnSize.height());
    dlg->updateContent();
    dlg->move(pos2);
    dlg->show();
    dlg->raise();
    dlg->activateWindow();
}

void NGSignInButton::onUserInfoUpdated()
{
    if(NGAccess::instance().isUserAuthorized()) {
        setIcon(NGAccess::instance().avatar());
    }
    else {
        setIcon(QIcon(":/icons/person-blue.svg"));
    }
    emit userInfoUpdated();
}
