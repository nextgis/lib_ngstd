/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Framework library
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

#include "signbutton.h"
#include "signdialog.h"

#include <QIcon>
#include <QPainter>
#include <QPushButton>

// NOTE: If AuthSourceType is custom, before create NGSignInButton must execute
//       NGAccess::instance().setAuthEndpoint, NGAccess::instance().setTokenEndpoint and
//       NGAccess::instance().setUserInfoEndpoint
NGSignInButton::NGSignInButton(const QString &clientId,
                               const QString &scope,
                               const QString &endPoint,
                               NGAccess::AuthSourceType authType,
                               QWidget * parent) :
    QToolButton (parent)
{
    NGAccess::instance().setEndPoint(endPoint, authType);
    NGAccess::instance().setScope(scope);
    NGAccess::instance().setClientId(clientId);

    QTimer::singleShot(500, [] () {
        NGAccess::instance().checkEndpointAsync();
    });
    onUserInfoUpdated();

    connect(this, SIGNAL(clicked()), this, SLOT(onClick()));
    connect(&NGAccess::instance(), SIGNAL(userInfoUpdated()), this, SLOT(onUserInfoUpdated()));
    connect(&NGAccess::instance(), SIGNAL(supportInfoUpdated()), this, SIGNAL(supportInfoUpdated()));

    m_signDialog = new NGSignDialog(this);
}

void NGSignInButton::onClick()
{   
    NGSignDialog *dlg = qobject_cast<NGSignDialog*>(m_signDialog);
    QSize dlgSize = dlg->size();
    QSize btnSize = size();
    QPoint pos1 = mapToGlobal(QPoint(btnSize.width(), btnSize.height()));
    QPoint pos2(pos1.x() - dlgSize.width(), pos1.y());
    if(pos2.x() < 0) {
        pos2.setX(pos1.x() - btnSize.width());
    }
    dlg->updateContent();
    dlg->move(pos2);
    dlg->show();
    dlg->raise();
    dlg->activateWindow();
}

void NGSignInButton::onUserInfoUpdated()
{
    QIcon userIcon = (NGAccess::instance().isUserAuthorized() ?
                          NGAccess::instance().avatar() :
                          QIcon(":/icons/person-blue.svg") );

    if (!NGAccess::instance().isEndpointAvailable()) {
        QPixmap pixmap = userIcon.pixmap(userIcon.actualSize(QSize(64, 64)));
        QPainter painter(&pixmap);
        painter.setPen(Qt::red);
        painter.setBrush(Qt::red);
        painter.drawEllipse(0, 0, 32, 32);
        userIcon = QIcon(pixmap);
    }

    setIcon(userIcon);

    emit userInfoUpdated();
}
