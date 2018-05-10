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
#ifndef NGFRAMEWORK_ACCESS_H
#define NGFRAMEWORK_ACCESS_H

#include "framework/framework.h"

#include <QDateTime>
#include <QFutureWatcher>
#include <QIcon>
#include <QObject>

class NGFRAMEWORK_EXPORT NGAccess : public QObject
{
    Q_OBJECT
public:
    static NGAccess& instance();
    void authorize();
    void exit();
    void save();
    bool isFunctionAvailable(const QString &app, const QString &func) const;
    bool isUserSupported() const;
    bool isUserAuthorized() const;

    void setScope(const QString &scope);
    void setClientId(const QString &clientId);

    QIcon avatar() const;
    QString avatarFilePath() const;
    QString firstName() const;
    QString lastName() const;

signals:
    void userInfoUpdated();
    void supportInfoUpdated();

private slots:
    void onUserInfoUpdated();
    void onSupportInfoUpdated();

private:
    NGAccess();
    virtual ~NGAccess() = default;
    NGAccess(NGAccess const&) = delete;
    NGAccess& operator= (NGAccess const&) = delete;

    bool checkSupported();
    bool verifyRSASignature(unsigned char *originalMessage, unsigned int messageLength,
                            unsigned char *signature, unsigned sigLength) const;
    void getTokens(const QString &code, const QString &redirectUri);
    void updateUserInfo() const;
    void updateSupportInfo() const;

private:
    bool m_authorized;
    bool m_supported;
    QString m_clientId, m_scope;
    QIcon m_avatar;
    QString m_configDir;
    QFutureWatcher<void> *m_updateUserInfoWatcher, *m_updateSupportInfoWatcher;
    QString m_firstName, m_lastName;
};

#endif // NGFRAMEWORK_ACCESS_H
