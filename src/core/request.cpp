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

#include "request.h"

#include <QDebug>
#include <QFile>

#include "cpl_http.h"
#include "cpl_json.h"

#define Q_CONSTCHAR(x) x.toLatin1().data()

NGRequest::NGRequest() : m_connTimeout("15"),
    m_timeout("20"),
    m_maxRetry("3"),
    m_retryDelay("5")
{
}

char **NGRequest::baseOptions() const
{
    char **options = nullptr;
    options = CSLAddNameValue(options, "CONNECTTIMEOUT", Q_CONSTCHAR(m_connTimeout));
    options = CSLAddNameValue(options, "TIMEOUT", Q_CONSTCHAR(m_timeout));
    options = CSLAddNameValue(options, "MAX_RETRY", Q_CONSTCHAR(m_maxRetry));
    options = CSLAddNameValue(options, "RETRY_DELAY", Q_CONSTCHAR(m_retryDelay));
    return options;
}

bool NGRequest::addAuth(const QString &url, const QMap<QString, QString> &options)
{
    AuthInfo info;
    info.m_clientId = options["clientId"];
    info.m_accessToken = options["accessToken"];
    info.m_updateToken = options["updateToken"];
    info.m_tokenServer = options["tokenServer"];
    info.m_expiresIn = options["expiresIn"].toInt();
    info.m_lastCheck = 0;

    if(info.m_expiresIn == -1) {
        CPLJSONDocument fetchToken;
        QString postPayload = QString("grant_type=authorization_code&code=%1&redirect_uri=%2&client_id=%3")
                .arg(options["code"])
                .arg(options["redirectUri"])
                .arg(info.m_clientId);
        char **options = instance().baseOptions();
        options = CSLAddNameValue(options, "CUSTOMREQUEST", "POST");
        options = CSLAddNameValue(options, "POSTFIELDS", Q_CONSTCHAR(postPayload));

        time_t now = time(nullptr);
        bool result = fetchToken.LoadUrl(Q_CONSTCHAR(info.m_tokenServer), options);
        qDebug() << "Server: " << info.m_tokenServer << "options:" << postPayload;
        CSLDestroy(options);
        if(!result) {
            qDebug() << "Failed to get tokens";
            return false;
        }

        CPLJSONObject root = fetchToken.GetRoot();
        info.m_accessToken = QString::fromStdString(
                    root.GetString("access_token", Q_CONSTCHAR(info.m_accessToken)));
        info.m_updateToken = QString::fromStdString(
                    root.GetString("refresh_token", Q_CONSTCHAR(info.m_updateToken)));
        info.m_expiresIn = root.GetInteger("expires_in", info.m_expiresIn);
        info.m_lastCheck = now;
    }

    instance().addAuth(url, info);
    return true;
}

QMap<QString, QVariant> NGRequest::getJsonAsMap(const QString &url)
{
    char **options = instance().baseOptions();
    QString headers = "Accept: */*";
    QString authHeaderStr = instance().authHeader(url);
    if(!authHeaderStr.isEmpty()) {
        headers += "\r\n" + authHeaderStr;
    }
    options = CSLAddNameValue(options, "HEADERS", Q_CONSTCHAR(headers));

    QMap<QString, QVariant> out;
    CPLJSONDocument in;
    if(in.LoadUrl(url.toStdString(), options)) {
        CPLJSONObject root = in.GetRoot();
        for(const CPLJSONObject &child : root.GetChildren()) {
            QString name = QString::fromStdString(child.GetName());
            switch(child.GetType()) {
            case CPLJSONObject::Boolean:
                out[name] = child.ToBool();
                break;
            case CPLJSONObject::String:
                out[name] = QString::fromStdString(child.ToString());
                break;
            case CPLJSONObject::Integer:
                out[name] = child.ToInteger();
                break;
            case CPLJSONObject::Long:
                out[name] = child.ToLong();
                break;
            case CPLJSONObject::Double:
                out[name] = child.ToDouble();
                break;
            default:
                out[name] = QString::fromStdString(child.ToString());
            }
        }
    }

    CSLDestroy(options);

    return out;
}

void NGRequest::getFile(const QString &url, const QString &path)
{
    char **options = instance().baseOptions();
    QString headers = "Accept: */*";
    QString authHeaderStr = instance().authHeader(url);
    if(!authHeaderStr.isEmpty()) {
        headers += "\r\n" + authHeaderStr;
    }
    options = CSLAddNameValue(options, "HEADERS", Q_CONSTCHAR(headers));
    CPLHTTPResult* result = CPLHTTPFetch(Q_CONSTCHAR(url), options);
    CSLDestroy(options);

    QByteArray data(reinterpret_cast<const char*>(result->pabyData), result->nDataLen);
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();

    CPLHTTPDestroyResult(result);
}

NGRequest &NGRequest::instance()
{
    static NGRequest n;
    return n;
}

void NGRequest::addAuth(const QString &url, NGRequest::AuthInfo auth)
{
    m_auths[url] = auth;
}

void NGRequest::removeAuth(const QString &url)
{
    m_auths.remove(url);
}

const QString NGRequest::authHeader(const QString &url)
{
    QMutexLocker locker(&m_mutex);
    QString header;
    QMap<QString, AuthInfo>::iterator it;
    for(it = m_auths.begin(); it != m_auths.end(); ++it) {
        if(url.startsWith(it.key())) {
            AuthInfo &info = it.value();

            // 1. Check if expires if not return current access token
            time_t now = time(nullptr);
            double seconds = difftime(now, info.m_lastCheck);
            if(seconds < info.m_expiresIn) {
                qDebug() << "Token is not expired. Url: " << url;
                return QString("Authorization: Bearer %1").arg(info.m_accessToken);
            }

            // 2. Try to update token
            // TODO: Get proxy from QNetworkProxy QNetworkAccessManager::proxy() const
            char **options = baseOptions();
            options = CSLAddNameValue(options, "CUSTOMREQUEST", "POST");
            options = CSLAddNameValue(options, "POSTFIELDS",
                                           CPLSPrintf("grant_type=refresh_token&client_id=%s&refresh_token=%s",
                                                      Q_CONSTCHAR(info.m_clientId),
                                                      Q_CONSTCHAR(info.m_updateToken)));

            CPLHTTPResult* result = CPLHTTPFetch(Q_CONSTCHAR(info.m_tokenServer),
                                                 options);
            CSLDestroy(options);

            if(result->nStatus != 0) {
                CPLHTTPDestroyResult( result );
                qDebug() << "Failed to refresh token. Return last not expired. Url: " << url;
                return QString("Authorization: Bearer %1").arg(info.m_accessToken);
            }

            CPLJSONDocument resultJson;
            if(!resultJson.LoadMemory(result->pabyData, result->nDataLen)) {
                CPLHTTPDestroyResult( result );
                qDebug() << "Token is expired. Url: " << url;
                return "expired";
            }
            CPLHTTPDestroyResult( result );

            // 4. Save new update and access tokens
            CPLJSONObject root = resultJson.GetRoot();
            if(!EQUAL(root.GetString("error", "").c_str(), "")) {
                qDebug() << "Token is expired. " <<
                            "\nError:" << QString::fromStdString(root.GetString("error", "")) <<
                            "\nUrl: " << url;
                return "expired";
            }

            info.m_accessToken = QString::fromStdString(
                        root.GetString("access_token", Q_CONSTCHAR(info.m_accessToken)));
            info.m_updateToken = QString::fromStdString(
                        root.GetString("refresh_token", Q_CONSTCHAR(info.m_updateToken)));
            info.m_expiresIn = root.GetInteger("expires_in", info.m_expiresIn);
            info.m_lastCheck = now;

            // 5. Return new Auth Header
            qDebug() << "Token updated. Url: " << url;

            return QString("Authorization: Bearer %1").arg(info.m_accessToken);
        }
    }
    return header;
}

const QMap<QString, QString> NGRequest::properties(const QString &url) const
{
    QMap<QString, QString> out;
    AuthInfo info = m_auths[url];
    out["clientId"] = info.m_clientId;
    out["accessToken"] = info.m_accessToken;
    out["updateToken"] = info.m_updateToken;
    out["tokenServer"] = info.m_tokenServer;
    out["expiresIn"] = QString::number(info.m_expiresIn);
    return out;
}
