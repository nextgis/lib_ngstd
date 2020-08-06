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

#include "request.h"

#ifdef Q_OS_WIN
#include <QCoreApplication>
#include <QDir>
#endif

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QMutex>
#include <QMutexLocker>
#include <QUrl>

#include "cpl_http.h"
#include "cpl_json.h"
#include "gdal.h"
#include "gdal_version.h"

#define Q_CONSTCHAR(x) x.toLatin1().data()

////////////////////////////////////////////////////////////////////////////////
// The HTTPAuthBasic class
////////////////////////////////////////////////////////////////////////////////

class HTTPAuthBasic : public IHTTPAuth {

public:
    explicit HTTPAuthBasic(const QString &login, const QString &password);
    virtual ~HTTPAuthBasic() override = default;
    virtual const QString header() override { return QString("Authorization: Basic %1").arg(m_basicAuth); }
    virtual const QMap<QString, QString> properties() const override;

private:
    QString m_basicAuth;
};

HTTPAuthBasic::HTTPAuthBasic(const QString &login, const QString &password)
{
    QByteArray str;
    str.append(login + ":" + password);
    m_basicAuth = str.toBase64();
}

const QMap<QString, QString> HTTPAuthBasic::properties() const
{
    QMap<QString, QString> out;
    out["type"] = "basic";
    out["basic"] = m_basicAuth;
    return out;
}

////////////////////////////////////////////////////////////////////////////////
// The HTTPAuthBearer class
////////////////////////////////////////////////////////////////////////////////
class HTTPAuthBearer : public IHTTPAuth {

public:
    explicit HTTPAuthBearer(const QString &clientId,
                            const QString &tokenServer, const QString &accessToken,
                            const QString &updateToken, int expiresIn,
                            time_t lastCheck, NGRequest *request);
    virtual ~HTTPAuthBearer() override = default;
    virtual const QString header() override;
    virtual const QMap<QString, QString> properties() const override;

private:
    QString m_clientId;
    QString m_accessToken;
    QString m_updateToken;
    QString m_tokenServer;
    int m_expiresIn;
    time_t m_lastCheck;
    NGRequest *m_request;
};

HTTPAuthBearer::HTTPAuthBearer(const QString &clientId,
                               const QString &tokenServer, const QString &accessToken,
                               const QString &updateToken, int expiresIn,
                               time_t lastCheck, NGRequest *request) : IHTTPAuth(),
    m_clientId(clientId),
    m_accessToken(accessToken),
    m_updateToken(updateToken),
    m_tokenServer(tokenServer),
    m_expiresIn(expiresIn),
    m_lastCheck(lastCheck),
    m_request(request)
{

}

const QMap<QString, QString> HTTPAuthBearer::properties() const
{
    QMap<QString, QString> out;
    out["type"] = "bearer";
    out["clientId"] = m_clientId;
    out["accessToken"] = m_accessToken;
    out["updateToken"] = m_updateToken;
    out["tokenServer"] = m_tokenServer;
    out["expiresIn"] = QString::number(m_expiresIn);
    return out;
}

const QString HTTPAuthBearer::header()
{
    // 1. Check if expires if not return current access token
    time_t now = time(nullptr);
    double seconds = difftime(now, m_lastCheck);
    if(seconds < m_expiresIn) {
        return QString("Authorization: Bearer %1").arg(m_accessToken);
    }

    // 2. Try to update token
    char **options = m_request->baseOptions();
    options = CSLAddNameValue(options, "CUSTOMREQUEST", "POST");
    options = CSLAddNameValue(options, "POSTFIELDS",
                              CPLSPrintf("grant_type=refresh_token&client_id=%s&refresh_token=%s",
                                         Q_CONSTCHAR(m_clientId),
                                         Q_CONSTCHAR(m_updateToken)));

    CPLHTTPResult *result = CPLHTTPFetch(Q_CONSTCHAR(m_tokenServer), options);
    CSLDestroy(options);

    if(result->nStatus != 0 || result->pszErrBuf != nullptr) {
        CPLHTTPDestroyResult( result );
        qDebug() << "Failed to refresh token. Return last not expired. ";
        return QString("Authorization: Bearer %1").arg(m_accessToken);
    }

    CPLJSONDocument resultJson;
    if(!resultJson.LoadMemory(result->pabyData, result->nDataLen)) {
        CPLHTTPDestroyResult( result );
        qDebug() << "Token is expired. ";
        return "expired";
    }
    CPLHTTPDestroyResult( result );

    // 4. Save new update and access tokens
    CPLJSONObject root = resultJson.GetRoot();
    if(!EQUAL(root.GetString("error", "").c_str(), "")) {
        qDebug() << "Token is expired. " <<
                    "\nError:" << QString::fromStdString(root.GetString("error", ""));
        return "expired";
    }

    m_accessToken = QString::fromStdString(
                root.GetString("access_token", Q_CONSTCHAR(m_accessToken)));
    m_updateToken = QString::fromStdString(
                root.GetString("refresh_token", Q_CONSTCHAR(m_updateToken)));
    m_expiresIn = root.GetInteger("expires_in", m_expiresIn);
    m_lastCheck = now;

    // 5. Return new Auth Header
    qDebug() << "Token updated.";

    return QString("Authorization: Bearer %1").arg(m_accessToken);
}

////////////////////////////////////////////////////////////////////////////////
// NGRequest
////////////////////////////////////////////////////////////////////////////////

QString NGRequest::m_detailed_error = "";

NGRequest::NGRequest() : m_connTimeout("15"),
    m_timeout("20"),
    m_maxRetry("3"),
    m_retryDelay("5")
{
#ifdef Q_OS_WIN
    // Add SSL cert path
    const QString &certPemPath = QCoreApplication::applicationDirPath() + QDir::separator() + QLatin1String("..\\share\\ssl\\certs");
    QDir certPemDir(certPemPath);
    m_certPem = certPemDir.absoluteFilePath("cert.pem");
#endif
}

NGRequest::~NGRequest()
{
}

char **NGRequest::baseOptions() const
{
    char **options = nullptr;
    options = CSLAddNameValue(options, "CONNECTTIMEOUT", Q_CONSTCHAR(m_connTimeout));
    options = CSLAddNameValue(options, "TIMEOUT", Q_CONSTCHAR(m_timeout));
    options = CSLAddNameValue(options, "MAX_RETRY", Q_CONSTCHAR(m_maxRetry));
    options = CSLAddNameValue(options, "RETRY_DELAY", Q_CONSTCHAR(m_retryDelay));

#ifdef Q_OS_WIN
    options = CSLAddNameValue(options, "CAINFO", Q_CONSTCHAR(m_certPem));
#endif

    return options;
}

bool NGRequest::addAuth(const QStringList &urls, const QMap<QString, QString> &options)
{
    if(options["type"] == "bearer") {
        int expiresIn = options["expiresIn"].toInt();
        QString clientId = options["clientId"];
        QString tokenServer = options["tokenServer"];
        QString accessToken = options["accessToken"];
        QString updateToken = options["updateToken"];
        time_t lastCheck = 0;
        if(expiresIn == -1) {
            CPLJSONDocument fetchToken;
            QString postPayload = QString("grant_type=authorization_code&code=%1&redirect_uri=%2&client_id=%3")
                    .arg(options["code"])
                    .arg(options["redirectUri"])
                    .arg(clientId);
            char **options = instance().baseOptions();
            options = CSLAddNameValue(options, "CUSTOMREQUEST", "POST");
            options = CSLAddNameValue(options, "POSTFIELDS", Q_CONSTCHAR(postPayload));

            time_t now = time(nullptr);
            bool result = fetchToken.LoadUrl(Q_CONSTCHAR(tokenServer), options);
            // qDebug() << "Server: " << info.m_tokenServer << "options:" << postPayload;
            CSLDestroy(options);
            if(!result) {
                qDebug() << "Failed to get tokens";
                return false;
            }

            CPLJSONObject root = fetchToken.GetRoot();
            accessToken = QString::fromStdString(
                        root.GetString("access_token", Q_CONSTCHAR(accessToken)));
            updateToken = QString::fromStdString(
                        root.GetString("refresh_token", Q_CONSTCHAR(updateToken)));
            expiresIn = root.GetInteger("expires_in", expiresIn);
            lastCheck = now;
        }

        HTTPAuthBearer *auth = new HTTPAuthBearer(clientId, tokenServer,
                                                  accessToken, updateToken,
                                                  expiresIn, lastCheck,
                                                  &instance());
        QSharedPointer<IHTTPAuth> authPtr(auth);
        foreach(const QString &url, urls) {
            instance().addAuth(url, authPtr);
        }
        return true;
    }
    return false;
}

QString NGRequest::getAsString(const QString &url)
{
    char **options = instance().baseOptions();
    QString headers = "Accept: */*";
    QString authHeaderStr = instance().authHeader(url);
    if(!authHeaderStr.isEmpty()) {
        headers += "\r\n" + authHeaderStr;
    }
    options = CSLAddNameValue(options, "HEADERS", Q_CONSTCHAR(headers));
    CPLHTTPResult *result = CPLHTTPFetch(Q_CONSTCHAR(url), options);
    CSLDestroy(options);
    if(result->nStatus != 0 || result->pszErrBuf != nullptr) {
        CPLHTTPDestroyResult( result );
        return QString();
    }
    QByteArray data(reinterpret_cast<const char*>(result->pabyData), result->nDataLen);
    CPLHTTPDestroyResult(result);
    return data;
}

QString NGRequest::getJsonAsString(const QString &url)
{
    char **options = instance().baseOptions();
    QString headers = "Accept: */*";
    QString authHeaderStr = instance().authHeader(url);
    if(!authHeaderStr.isEmpty()) {
        headers += "\r\n" + authHeaderStr;
    }
    options = CSLAddNameValue(options, "HEADERS", Q_CONSTCHAR(headers));
    QString out;
    CPLJSONDocument in;
    if(in.LoadUrl(url.toStdString(), options)) {
        out = QString::fromStdString(in.SaveAsString());
    }
    CSLDestroy(options);

    return out;
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

bool NGRequest::getFile(const QString &url, const QString &path)
{
    char **options = instance().baseOptions();
    QString headers = "Accept: */*";
    QString authHeaderStr = instance().authHeader(url);
    if(!authHeaderStr.isEmpty()) {
        headers += "\r\n" + authHeaderStr;
    }
    options = CSLAddNameValue(options, "HEADERS", Q_CONSTCHAR(headers));
    CPLHTTPResult *result = CPLHTTPFetch(Q_CONSTCHAR(url), options);
    CSLDestroy(options);

    if(result->nStatus != 0 || result->pszErrBuf != nullptr) {
        CPLHTTPDestroyResult( result );
        return false;
    }

    QByteArray data(reinterpret_cast<const char*>(result->pabyData), result->nDataLen);
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();

    CPLHTTPDestroyResult(result);

    return true;
}

NGRequest &NGRequest::instance()
{
    static NGRequest n;
    return n;
}

void NGRequest::addAuth(const QString &url, QSharedPointer<IHTTPAuth> auth)
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
    QMap<QString, QSharedPointer<IHTTPAuth>>::iterator it;
    for(it = m_auths.begin(); it != m_auths.end(); ++it) {
        if(url.startsWith(it.key())) {
            return it.value()->header();
        }
    }
    return header;
}

/**
 * @brief Auth class instance current properties. During request auth properties may change (for example, oAuth update and access tokens, etc.),
 * @param url URL auth class belongs to.
 * @return map of key - valuer auth properties.
 */
const QMap<QString, QString> NGRequest::properties(const QString &url) const
{
    QMap<QString, QString> out;
    if(m_auths.contains(url)) {
        return m_auths[url]->properties();
    }
    return out;
}

QString NGRequest::getAuthHeader(const QString &url)
{
    return instance().authHeader(url);
}

/**
 * @brief Upload file to specified url
 * @param url URL to upload file
 * @param path File path in OS
 * @param name Name in form
 * @return Empty string if error or upload output (usually json)
 */
QString NGRequest::uploadFile(const QString &url, const QString &path,
                              const QString &name)
{
    m_detailed_error = "";
    
    if(atoi(GDALVersionInfo("VERSION_NUM")) < GDAL_COMPUTE_VERSION(2,4,0) ) {
        // Upload files supported only in GDAL >= 2.4
        m_detailed_error = QString("Unsupported GDAL version = %1").arg(GDALVersionInfo("VERSION_NUM"));
        return "";
    }
    char **options = instance().baseOptions();
    QString headers = "Accept: */*";
    QString authHeaderStr = instance().authHeader(url);
    if(!authHeaderStr.isEmpty()) {
        headers += "\r\n" + authHeaderStr;
    }
    options = CSLAddNameValue(options, "HEADERS", Q_CONSTCHAR(headers));
    options = CSLAddNameValue(options, "FORM_FILE_PATH", Q_CONSTCHAR(path));
    options = CSLAddNameValue(options, "FORM_FILE_NAME", Q_CONSTCHAR(name));
    CPLHTTPResult *result = CPLHTTPFetch(Q_CONSTCHAR(url), options);
    CSLDestroy(options);

    if(result->nStatus != 0 || result->pszErrBuf != nullptr) {
        m_detailed_error = QString("CPLHTTPFetch() failed. Info: \nnStatus = %1 \npszErrBuf = %2 ")
            .arg(result->nStatus).arg(result->pszErrBuf == nullptr ? "" : result->pszErrBuf);
        CPLHTTPDestroyResult( result );
        return "";
    }

    QByteArray data(reinterpret_cast<const char*>(result->pabyData), result->nDataLen);
    CPLHTTPDestroyResult(result);

    return data;
}

/**
 * @brief NGRequest::setProxy Set proxy for all requests.
 * @param useProxy Use or not proxy.
 * @param useSystemProxy Get proxy information from system. Any other properties ignored.
 * @param proxyUrl Proxy url.
 * @param porxyPort Proxy port.
 * @param proxyUser User to authenticate in proxy.
 * @param proxyPassword Password to authenticate in proxy.
 * @param proxyAuth Proxy authentication scheme to use. Can be BASIC/NTLM/DIGEST/ANY.
 */
void NGRequest::setProxy(bool useProxy, bool useSystemProxy, const QString &proxyUrl,
                         int porxyPort, const QString &proxyUser,
                         const QString &proxyPassword, const QString &proxyAuth)
{
    if(useProxy) {
        std::string url;
        std::string userpwd;
        if(useSystemProxy) {
            QNetworkProxyQuery npq(QUrl("http://www.google.com"));
            QList<QNetworkProxy> listOfProxies =
                    QNetworkProxyFactory::systemProxyForQuery(npq);
            // Get first proxy if any.
            if(!listOfProxies.isEmpty()) {
                url = listOfProxies[0].hostName().toStdString() + ":" +
                        std::to_string(listOfProxies[0].port());
                userpwd = listOfProxies[0].user().toStdString() + ":" +
                        listOfProxies[0].password().toStdString();

            }
        }
        else {
            url = proxyUrl.toStdString() + ":" + std::to_string(porxyPort);
            userpwd = proxyUser.toStdString() + ":" +
                    proxyPassword.toStdString();
            CPLSetConfigOption("GDAL_PROXY_AUTH", proxyAuth.toStdString().c_str());
        }
        CPLSetConfigOption("GDAL_HTTP_PROXY", url.c_str());
        CPLSetConfigOption("GDAL_HTTP_PROXYUSERPWD", userpwd.c_str());
    }
    else {
        CPLSetConfigOption("GDAL_HTTP_PROXY", nullptr);
        CPLSetConfigOption("GDAL_HTTP_PROXYUSERPWD", nullptr);
        CPLSetConfigOption("GDAL_PROXY_AUTH", nullptr);
    }
}
