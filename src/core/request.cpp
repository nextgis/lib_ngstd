/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Core Library
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

// std
#include <array>

#include "core/util.h"

static CPLStringList getOptions(const QString &url) {
    CPLStringList options(NGRequest::instance().baseOptions());
    QString headers = "Accept: */*";
    options.AddNameValue("HEADERS", headers.toStdString().c_str());
    return options;
}

static QRecursiveMutex gMutex;    

////////////////////////////////////////////////////////////////////////////////
// Authorization header callback
////////////////////////////////////////////////////////////////////////////////

static auto gAuthHeaderCallback = [](const char *pszURL) -> std::string
{
    if (!pszURL)
        return "";
    return NGRequest::instance().authHeader(QString(pszURL)).toStdString();
};

static void InstallAuthHeaderCallback()
{
    CPLHTTPSetAuthHeaderCallback(gAuthHeaderCallback);
}

static void RemoveAuthHeaderCallback()
{
    CPLHTTPSetAuthHeaderCallback(nullptr);
}

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
    seconds += 2; // Two seconds addition to expiration
    if(seconds < m_expiresIn) {
        return QString("Authorization: Bearer %1").arg(m_accessToken);
    }

    // 2. Try to update token
    auto updateToken = m_updateToken.toStdString();
    const char *payload = CPLSPrintf("grant_type=refresh_token&client_id=%s&refresh_token=%s",
                                m_clientId.toStdString().c_str(),
                                updateToken.c_str());
    CPLStringList options(m_request->baseOptions());
    options.AddNameValue("CUSTOMREQUEST", "POST");
    options.AddNameValue("POSTFIELDS", payload);

    RemoveAuthHeaderCallback();

    CPLHTTPResult *result = CPLHTTPFetch(m_tokenServer.toStdString().c_str(), options);

    InstallAuthHeaderCallback();

    if(result->nStatus != 0 || result->pszErrBuf != nullptr) {
        if(EQUALN("HTTP error code :", result->pszErrBuf, 17) == FALSE) { // If server error refresh token - logout
            CPLHTTPDestroyResult( result );
            qDebug() << "Failed to refresh token. Return last not expired. ";
            return QString("Authorization: Bearer %1").arg(m_accessToken);
        }
    }

    m_accessToken.clear();
    CPLJSONDocument resultJson;
    if(!resultJson.LoadMemory(result->pabyData, result->nDataLen)) {
        CPLHTTPDestroyResult( result );
        qDebug() << "Token is expired. ";
        return "expired";
    }
    CPLHTTPDestroyResult( result );

    // 4. Save new update and access tokens
    CPLJSONObject root = resultJson.GetRoot();
    auto err = root.GetString("error", "");
    if(!err.empty()) {
        qDebug() << "Token is expired. " <<
                    "\nError:" << QString::fromStdString(err);
        return "expired";
    }

    m_accessToken = QString::fromStdString(
                root.GetString("access_token", m_accessToken.toStdString()));
    m_updateToken = QString::fromStdString(
                root.GetString("refresh_token", updateToken));
    m_expiresIn = root.GetInteger("expires_in", m_expiresIn);
    m_lastCheck = now;

    // 5. Return new Auth Header
    qDebug() << "Token updated.";

    return QString("Authorization: Bearer %1").arg(m_accessToken);
}

////////////////////////////////////////////////////////////////////////////////
// NGRequest
////////////////////////////////////////////////////////////////////////////////

NGRequest::NGRequest() :
    m_connTimeout("15"),
    m_timeout("20"),
    m_maxRetry("3"),
    m_retryDelay("5"),
    m_detailedError("")
{
    InstallAuthHeaderCallback();

#ifdef Q_OS_WIN
    // Add SSL cert path
    const QString &certPemPath = QCoreApplication::applicationDirPath() + QDir::separator() + QLatin1String("..\\share\\ssl\\certs");
    QDir certPemDir(certPemPath);
    m_certPem = certPemDir.absoluteFilePath("cert.pem");
#endif
}

NGRequest::~NGRequest()
{
    RemoveAuthHeaderCallback();
}

void NGRequest::setErrorMessage(const QString &err)
{
    m_detailedError = err;
}

char **NGRequest::baseOptions() const
{
    char **options = nullptr;
    auto connTimeout = m_connTimeout.toStdString();
    options = CSLAddNameValue(options, "CONNECTTIMEOUT", connTimeout.c_str());
    auto timeout = m_timeout.toStdString();
    options = CSLAddNameValue(options, "TIMEOUT", timeout.c_str());
    auto maxRetry = m_maxRetry.toStdString();
    options = CSLAddNameValue(options, "MAX_RETRY", maxRetry.c_str());
    auto retryDelay = m_retryDelay.toStdString();
    options = CSLAddNameValue(options, "RETRY_DELAY", retryDelay.c_str());

#ifdef Q_OS_WIN
    auto certPem = m_certPem.toStdString();
    options = CSLAddNameValue(options, "CAINFO", certPem.c_str());
#endif

    return options;
}

QString NGRequest::lastError() const
{
    return m_detailedError;
}

void NGRequest::resetError()
{
    m_detailedError.clear();
}

bool NGRequest::addAuth(const QStringList &urls, const QMap<QString, QString> &options)
{
    QMutexLocker locker(&gMutex);

    if(options["type"] == "bearer") {
        int expiresIn = options["expiresIn"].toInt();
        QString clientId = options["clientId"];
        QString tokenServer = options["tokenServer"];
        QString accessToken = options["accessToken"];
        QString updateToken = options["updateToken"];
        QString verify = options["codeVerifier"];
        time_t lastCheck = 0;
        if(expiresIn == -1) {
            CPLJSONDocument fetchToken;
            QString postPayload = QString("grant_type=authorization_code&code=%1&redirect_uri=%2&client_id=%3")
                    .arg(options["code"])
                    .arg(options["redirectUri"])
                    .arg(clientId);
            if(!verify.isEmpty()) {
                postPayload += "&code_verifier=" + verify;
            }
            CPLStringList options(instance().baseOptions());
            auto payload = postPayload.toStdString();
            options.AddNameValue("CUSTOMREQUEST", "POST");
            options.AddNameValue("POSTFIELDS", payload.c_str());

            time_t now = time(nullptr);
            auto tokenServerStd = tokenServer.toStdString();
            bool result = fetchToken.LoadUrl(tokenServerStd.c_str(), options);
            qDebug() << "Server: " << tokenServer << "\noptions:" << postPayload;
            if(!result) {
                qDebug() << "Failed to get tokens";
                return false;
            }

            CPLJSONObject root = fetchToken.GetRoot();
            auto accessTokenStd = accessToken.toStdString();
            accessToken = QString::fromStdString(
                        root.GetString("access_token", accessTokenStd));
            auto updateTokenStr = updateToken.toStdString();
            updateToken = QString::fromStdString(
                        root.GetString("refresh_token", updateTokenStr.c_str()));
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
    QMutexLocker locker(&gMutex);

    CPLStringList options = getOptions(url);
    CPLHTTPResult *result = CPLHTTPFetch(url.toStdString().c_str(), options);
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
    QMutexLocker locker(&gMutex);

    CPLStringList options = getOptions(url);
    QString out;
    CPLJSONDocument in;
    if(in.LoadUrl(url.toStdString(), options)) {
        out = QString::fromUtf8(in.SaveAsString().c_str());
    }
    return out;
}

QMap<QString, QVariant> NGRequest::getJsonAsMap(const QString &url)
{
    QMutexLocker locker(&gMutex);

    CPLStringList options = getOptions(url);
    CPLJSONDocument in;
    if(in.LoadUrl(url.toStdString(), options)) {
        qDebug() << QString::fromStdString(in.GetRoot().Format(CPLJSONObject::PrettyFormat::Pretty));
        return toMap(in.GetRoot());
    }

    return QMap<QString, QVariant>();
}

bool NGRequest::getFile(const QString &url, const QString &path)
{
    QMutexLocker locker(&gMutex);

    CPLStringList options = getOptions(url);
    CPLHTTPResult *result = CPLHTTPFetch(url.toStdString().c_str(), options);
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

void NGRequest::removeAuth(const QString &url, const QString &logoutUrl)
{
    QMutexLocker locker(&gMutex);

    if(!logoutUrl.isEmpty()) {
        auto prop = properties(url);
        if(!prop.empty()) {
            auto updateToken = prop["updateToken"].toStdString();
            auto clientId = prop["clientId"].toStdString();
            const char *payload = CPLSPrintf("client_id=%s&refresh_token=%s",
                                    clientId.c_str(),
                                    updateToken.c_str());
            CPLStringList options(baseOptions());
            options.AddNameValue("CUSTOMREQUEST", "POST");
            options.AddNameValue("POSTFIELDS", payload);
            options.AddNameValue("HEADERS", "Content-Type: application/x-www-form-urlencoded");

            CPLHTTPResult *result = CPLHTTPFetch(logoutUrl.toStdString().c_str(), options);

            if(result->nStatus != 0 || result->pszErrBuf != nullptr) {
                if(EQUALN("HTTP error code :", result->pszErrBuf, 17) == FALSE) { // If server error refresh token - logout
                    qDebug() << "Failed to logout.";
                }
            }
            CPLHTTPDestroyResult( result );
        }
    }
    m_auths.remove(url);
}

const QString NGRequest::authHeader(const QString &url)
{
    QMutexLocker locker(&gMutex);

    if(!m_auths.empty() && url == "any") {
        auto it = m_auths.constBegin();
        return it.value()->header();
    }
    QMap<QString, QSharedPointer<IHTTPAuth>>::iterator it;
    for(it = m_auths.begin(); it != m_auths.end(); ++it) {
        if(url.startsWith(it.key())) {
            return it.value()->header();
        }
    }
    return QString();
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
    QMutexLocker locker(&gMutex);

    CPLErrorReset();
    instance().resetError();
    
    if(atoi(GDALVersionInfo("VERSION_NUM")) < GDAL_COMPUTE_VERSION(2,4,0) ) {
        // Upload files supported only in GDAL >= 2.4
        instance().setErrorMessage(
                    QString("Unsupported GDAL version = %1").arg(
                        GDALVersionInfo("VERSION_NUM")));
        return "";
    }
    CPLStringList options = getOptions(url);
    options.AddNameValue("FORM_FILE_PATH", path.toStdString().c_str());
    options.AddNameValue("FORM_FILE_NAME", name.toStdString().c_str());
    CPLHTTPResult *result = CPLHTTPFetch(url.toStdString().c_str(), options);
    if(result->nStatus != 0 || result->pszErrBuf != nullptr) {
        instance().setErrorMessage(
                    QString("CPLHTTPFetch() failed. Info: \nnStatus = %1 \npszErrBuf = %2 \nGDAL error = %3")
            .arg(result->nStatus).arg(result->pszErrBuf == nullptr ? "" : result->pszErrBuf).arg(CPLGetLastErrorMsg()));
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
