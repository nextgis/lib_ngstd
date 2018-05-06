/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Core Library
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
#ifndef NGCORE_REQUEST_H
#define NGCORE_REQUEST_H

#include "core/core.h"

#include <QMap>
#include <QMutex>
#include <QVariant>

class NGCORE_EXPORT NGRequest
{

public:
    static bool addAuth(const QString &url, const QMap<QString, QString> &options);
    static QMap<QString, QVariant> getJsonAsMap(const QString &url);
    static void getFile(const QString &url, const QString &path);
    static NGRequest& instance();

    typedef struct _authInfo {
        QString m_clientId;
        QString m_accessToken;
        QString m_updateToken;
        QString m_tokenServer;
        int m_expiresIn;
        time_t m_lastCheck;
    } AuthInfo;
public:
    void addAuth(const QString &url, AuthInfo auth);
    void removeAuth(const QString &url);
    const QString authHeader(const QString &url);
    const QMap<QString, QString> properties(const QString &url) const;


private:
    NGRequest();
    ~NGRequest() = default;
    NGRequest(NGRequest const&) = delete;
    NGRequest& operator= (NGRequest const&) = delete;
    char **baseOptions() const;

private:
    QMap<QString, AuthInfo> m_auths;
    QString m_connTimeout;
    QString m_timeout;
    QString m_maxRetry;
    QString m_retryDelay;
    QMutex m_mutex;
};


#endif // NGCORE_REQUEST_H
