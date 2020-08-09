/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Core Library
*  Author:  Dmitry Baryshnikov, bishop.dev@gmail.com
*******************************************************************************
*  Copyright (C) 2012-2019 NextGIS, info@nextgis.ru
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
#include <QSharedPointer>
#include <QStringList>
#include <QVariant>

/**
 * @brief The IHTTPAuth class is base class for HTTP Authorization headers
 */
class IHTTPAuth {
public:
    virtual ~IHTTPAuth() = default;
    virtual const QString header() = 0;
    virtual const QMap<QString, QString> properties() const = 0;
};

class NGCORE_EXPORT NGRequest
{

public:
    static bool addAuth(const QStringList &urls, const QMap<QString, QString> &options);
    static QMap<QString, QVariant> getJsonAsMap(const QString &url);
    static QString getJsonAsString(const QString &url);
    static QString getAsString(const QString &url);
    static bool getFile(const QString &url, const QString &path);
    static QString getAuthHeader(const QString &url);
    static QString uploadFile(const QString &url, const QString &path,
                              const QString &name);
    static void setProxy(bool useProxy = true, bool useSystemProxy = true,
                         const QString &proxyUrl = "",
                         int proxyPort = 0, const QString &proxyUser = "",
                         const QString &proxyPassword = "",
                         const QString &proxyAuth = "ANY");
    static NGRequest &instance();

public:
    void addAuth(const QString &url, QSharedPointer<IHTTPAuth> auth);
    void removeAuth(const QString &url);
    const QString authHeader(const QString &url);
    const QMap<QString, QString> properties(const QString &url) const;
    char **baseOptions() const;
    QString lastError() const;
    void resetError();

protected:
    NGRequest();
    ~NGRequest();
    NGRequest(const NGRequest &) = delete;
    NGRequest &operator= (const NGRequest &) = delete;
    void setErrorMessage(const QString &err);

private:
    QMap<QString, QSharedPointer<IHTTPAuth>> m_auths;
    QString m_connTimeout;
    QString m_timeout;
    QString m_maxRetry;
    QString m_retryDelay;
    QString m_certPem;
    QMutex m_mutex;    
    QString m_detailedError;
};


#endif // NGCORE_REQUEST_H
