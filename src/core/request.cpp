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
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslError>
#include <QTimer>
#include <QUrl>
#include <QtGlobal>

#include "cpl_http.h"
#include "cpl_string.h"
#include "gdal.h"
#include "gdal_version.h"

// std
#include <algorithm>
#include <array>

#include "core/util.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#   include <mutex>
    static std::recursive_mutex gMutex;
#   define MUTEX_LOCKER std::lock_guard<std::recursive_mutex> locker(gMutex)
#else
#   include <QRecursiveMutex>
#   include <QMutexLocker>
    static QRecursiveMutex gMutex;    
#   define MUTEX_LOCKER QMutexLocker locker(&gMutex)
#endif


namespace {

struct RequestResult {
    QByteArray data;
    int httpStatus = 0;
    QNetworkReply::NetworkError error = QNetworkReply::NoError;
    QString errorString;
    bool timedOut = false;
};

bool requestFailed(const RequestResult &result)
{
    return result.error != QNetworkReply::NoError ||
           result.timedOut ||
           result.httpStatus >= 400;
}

QNetworkAccessManager *networkManager()
{
    static QNetworkAccessManager manager;
    return &manager;
}

void applyHeaders(QNetworkRequest &request, const QString &url, bool useAuthHeader,
                  const QByteArray &contentType)
{
    request.setRawHeader("Accept", "*/*");

    if (useAuthHeader) {
        const QString authHeader = NGRequest::getAuthHeader(url).trimmed();
        if (!authHeader.isEmpty()) {
            const int sep = authHeader.indexOf(':');
            if (sep > 0) {
                const QByteArray name = authHeader.left(sep).trimmed().toUtf8();
                const QByteArray value = authHeader.mid(sep + 1).trimmed().toUtf8();
                request.setRawHeader(name, value);
            }
        }
    }

    if (!contentType.isEmpty()) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    }
}

RequestResult executeRequest(const QString &url, const QString &method,
                             const QByteArray &body, const QByteArray &contentType,
                             bool useAuthHeader, QHttpMultiPart *multiPart,
                             int timeoutMs)
{
    RequestResult result;
    QNetworkRequest request{QUrl(url)};
    applyHeaders(request, url, useAuthHeader, contentType);

    const QByteArray requestMethod = method.trimmed().toUpper().toUtf8();
    auto setProtocolError = [&result](const QString &message) {
        result.error = QNetworkReply::ProtocolInvalidOperationError;
        result.errorString = message;
    };
    QNetworkReply *reply = nullptr;
    if (multiPart && requestMethod != "POST") {
        setProtocolError(QStringLiteral("Multipart payload is only supported for POST"));
        return result;
    }

    if (requestMethod == "POST") {
        if (multiPart) {
            reply = networkManager()->post(request, multiPart);
            // Ensure multipart lifetime is tied to reply lifecycle.
            multiPart->setParent(reply);
        } else {
            reply = networkManager()->post(request, body);
        }
    }
    else if (requestMethod == "GET") {
        reply = networkManager()->get(request);
    }
    else if (requestMethod == "PUT") {
        reply = networkManager()->put(request, body);
    }
    else if (requestMethod == "DELETE") {
        if (body.isEmpty()) {
            reply = networkManager()->deleteResource(request);
        }
        else {
            reply = networkManager()->sendCustomRequest(request, "DELETE", body);
        }
    }
    else if (requestMethod == "PATCH") {
        reply = networkManager()->sendCustomRequest(request, "PATCH", body);
    }
    else if (requestMethod == "HEAD") {
        reply = networkManager()->head(request);
    }
    else {
        setProtocolError(QStringLiteral("Unsupported HTTP method: %1").arg(method));
        return result;
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::sslErrors, reply,
                     [reply](const QList<QSslError> &errors) {
        bool onlySelfSignedRelated = true;
        bool hasSelfSignedError = false;
        for (const auto &err : errors) {
            const auto type = err.error();
            if (type == QSslError::SelfSignedCertificate ||
               type == QSslError::SelfSignedCertificateInChain) {
                hasSelfSignedError = true;
                continue;
            }
            if (type != QSslError::CertificateUntrusted) {
                onlySelfSignedRelated = false;
                break;
            }
        }
        if (onlySelfSignedRelated && hasSelfSignedError) {
            reply->ignoreSslErrors(errors);
        }
    });

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(timeoutMs);
    loop.exec();

    if (timer.isActive()) {
        timer.stop();
    }
    else {
        result.timedOut = true;
        reply->abort();
    }

    result.error = reply->error();
    result.errorString = reply->errorString();
    result.httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    result.data = reply->readAll();
    reply->deleteLater();
    return result;
}

void waitMs(int delayMs)
{
    if (delayMs <= 0) {
        return;
    }
    QEventLoop loop;
    QTimer::singleShot(delayMs, &loop, &QEventLoop::quit);
    loop.exec();
}

RequestResult requestWithRetries(const QString &url, const QString &method,
                                 const QByteArray &body, const QByteArray &contentType,
                                 bool useAuthHeader, QHttpMultiPart *multiPart,
                                 int timeoutMs, int maxRetry, int retryDelayMs)
{
    RequestResult result;
    // QHttpMultiPart is consumable payload; retries would reuse invalid data.
    const int attemptsLimit = multiPart ? 0 : maxRetry;
    for (int attempt = 0; attempt <= attemptsLimit; ++attempt) {
        result = executeRequest(url, method, body, contentType, useAuthHeader,
                                multiPart, timeoutMs);
        if (result.error == QNetworkReply::NoError && !result.timedOut) {
            return result;
        }
        if (attempt < attemptsLimit) {
            waitMs(retryDelayMs);
        }
    }
    return result;
}

bool parseJsonObject(const QByteArray &data, QJsonObject *out, QString *error)
{
    QJsonParseError parseError{};
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (error) {
            *error = parseError.errorString();
        }
        return false;
    }
    if (!doc.isObject()) {
        if (error) {
            *error = QStringLiteral("JSON is not an object");
        }
        return false;
    }
    if (out) {
        *out = doc.object();
    }
    return true;
}

std::pair<std::string, std::string> resolveGDALProxyCredentials(const bool useSystemProxy, const QString &proxyUrl,
                                                                const int proxyPort, const QString& proxyUser,
                                                                const QString& proxyPassword)
{
    const auto makeUserPwd = [](const QString &user, const QString &password) -> std::string
    {
        if (user.isEmpty() && password.isEmpty()) {
            return std::string();
        }
        return user.toStdString() + ":" + password.toStdString();
    };

    std::string url;
    std::string userpwd;

    if (useSystemProxy) {
        QNetworkProxyQuery npq(QUrl("http://www.google.com"));
        QList<QNetworkProxy> listOfProxies =
            QNetworkProxyFactory::systemProxyForQuery(npq);
        // Get first proxy if any.
        if (!listOfProxies.isEmpty()) {
            url = listOfProxies[0].hostName().toStdString() + ":" +
                std::to_string(listOfProxies[0].port());
            userpwd = makeUserPwd(listOfProxies[0].user(),
                                  listOfProxies[0].password());

        }
    }
    else
    {
        url = proxyUrl.toStdString() + ":" + std::to_string(proxyPort);
        userpwd = makeUserPwd(proxyUser, proxyPassword);
    }

    return std::make_pair(url, userpwd);
}

} // namespace

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
    if (seconds < m_expiresIn) {
        return QString("Authorization: Bearer %1").arg(m_accessToken);
    }

    // 2. Try to update token
    const QString payload = QString("grant_type=refresh_token&client_id=%1&refresh_token=%2")
            .arg(QString::fromUtf8(QUrl::toPercentEncoding(m_clientId)),
                 QString::fromUtf8(QUrl::toPercentEncoding(m_updateToken)));
    const int timeoutMs = std::max(1, m_request->timeout()) * 1000;
    const int maxRetry = std::max(0, m_request->maxRetry());
    const int retryDelayMs = std::max(0, m_request->retryDelay()) * 1000;

    RequestResult result = requestWithRetries(m_tokenServer, "POST",
                                              payload.toUtf8(),
                                              "application/x-www-form-urlencoded",
                                              false, nullptr, timeoutMs,
                                              maxRetry, retryDelayMs);

    if (requestFailed(result)) {
        qDebug() << "Failed to refresh token. Return last not expired. ";
        return QString("Authorization: Bearer %1").arg(m_accessToken);
    }

    m_accessToken.clear();
    QJsonObject root;
    QString parseError;
    if (!parseJsonObject(result.data, &root, &parseError)) {
        qDebug() << "Token is expired. " << "\nError:" << parseError;
        return "expired";
    }

    const QString err = root.value("error").toString();
    if (!err.isEmpty()) {
        qDebug() << "Token is expired. " << "\nError:" << err;
        return "expired";
    }

    m_accessToken = root.value("access_token").toString(m_accessToken);
    m_updateToken = root.value("refresh_token").toString(m_updateToken);
    m_expiresIn = root.value("expires_in").toInt(m_expiresIn);
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
    networkManager();

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
    MUTEX_LOCKER;
    m_detailedError = err;
}

char **NGRequest::baseOptions() const
{
    MUTEX_LOCKER;
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
    MUTEX_LOCKER;
    return m_detailedError;
}

void NGRequest::resetError()
{
    MUTEX_LOCKER;
    m_detailedError.clear();
}

int NGRequest::timeout() const
{
    MUTEX_LOCKER;
    return m_timeout.toInt();
}

int NGRequest::maxRetry() const
{
    MUTEX_LOCKER;
    return m_maxRetry.toInt();
}

int NGRequest::retryDelay() const
{
    MUTEX_LOCKER;
    return m_retryDelay.toInt();
}

bool NGRequest::addAuth(const QStringList &urls, const QMap<QString, QString> &options)
{
    MUTEX_LOCKER;

    if (options["type"] == "bearer") {
        int expiresIn = options["expiresIn"].toInt();
        QString clientId = options["clientId"];
        QString tokenServer = options["tokenServer"];
        QString accessToken = options["accessToken"];
        QString updateToken = options["updateToken"];
        QString verify = options["codeVerifier"];
        time_t lastCheck = 0;
        if (expiresIn == -1) {
            QString postPayload = QString("grant_type=authorization_code&code=%1&redirect_uri=%2&client_id=%3")
                    .arg(QString::fromUtf8(QUrl::toPercentEncoding(options["code"])))
                    .arg(QString::fromUtf8(QUrl::toPercentEncoding(options["redirectUri"])))
                    .arg(QString::fromUtf8(QUrl::toPercentEncoding(clientId)));
            if (!verify.isEmpty()) {
                postPayload += "&code_verifier=" + QString::fromUtf8(QUrl::toPercentEncoding(verify));
            }
            time_t now = time(nullptr);
            qDebug() << "Server: " << tokenServer << "\noptions:" << postPayload;
            const int timeoutMs = std::max(1, instance().m_timeout.toInt()) * 1000;
            const int maxRetry = std::max(0, instance().m_maxRetry.toInt());
            const int retryDelayMs = std::max(0, instance().m_retryDelay.toInt()) * 1000;
            RequestResult result = requestWithRetries(tokenServer, "POST",
                                                      postPayload.toUtf8(),
                                                      "application/x-www-form-urlencoded",
                                                      false, nullptr, timeoutMs,
                                                      maxRetry, retryDelayMs);
            if (requestFailed(result)) {
                qDebug() << "Failed to get tokens";
                return false;
            }

            QJsonObject root;
            QString parseError;
            if (!parseJsonObject(result.data, &root, &parseError)) {
                qDebug() << "Failed to parse token response: " << parseError;
                return false;
            }
            accessToken = root.value("access_token").toString(accessToken);
            updateToken = root.value("refresh_token").toString(updateToken);
            expiresIn = root.value("expires_in").toInt(expiresIn);
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
    else if (options["type"] == "basic") {
        QString login = options["login"];
        QString password = options["password"];

        QSharedPointer<IHTTPAuth> authPtr(new HTTPAuthBasic(login, password));
        foreach(const QString &url, urls) {
            instance().addAuth(url, authPtr);
        }
        return true;
    }

    return false;
}
 
bool NGRequest::addAuthURL(const QString &basicUrl, const QString &newUrl)
{
    return instance().addAuthURLImpl(basicUrl, newUrl);
}

void NGRequest::removeAuthURL(const QString &url)
{
    instance().removeAuthURLImpl(url);
}

QString NGRequest::getAsString(const QString &url)
{
    MUTEX_LOCKER;

    const int timeoutMs = std::max(1, instance().m_timeout.toInt()) * 1000;
    const int maxRetry = std::max(0, instance().m_maxRetry.toInt());
    const int retryDelayMs = std::max(0, instance().m_retryDelay.toInt()) * 1000;
    RequestResult result = requestWithRetries(url, "GET", QByteArray(),
                                              QByteArray(), true, nullptr,
                                              timeoutMs, maxRetry, retryDelayMs);
    if (requestFailed(result)) {
        return QString();
    }
    return QString::fromUtf8(result.data);
}

QString NGRequest::getJsonAsString(const QString &url)
{
    MUTEX_LOCKER;

    const int timeoutMs = std::max(1, instance().m_timeout.toInt()) * 1000;
    const int maxRetry = std::max(0, instance().m_maxRetry.toInt());
    const int retryDelayMs = std::max(0, instance().m_retryDelay.toInt()) * 1000;
    RequestResult result = requestWithRetries(url, "GET", QByteArray(),
                                              QByteArray(), true, nullptr,
                                              timeoutMs, maxRetry, retryDelayMs);
    if (requestFailed(result)) {
        return QString();
    }
    return QString::fromUtf8(result.data);
}

QMap<QString, QVariant> NGRequest::getJsonAsMap(const QString &url)
{
    MUTEX_LOCKER;

    const int timeoutMs = std::max(1, instance().m_timeout.toInt()) * 1000;
    const int maxRetry = std::max(0, instance().m_maxRetry.toInt());
    const int retryDelayMs = std::max(0, instance().m_retryDelay.toInt()) * 1000;
    RequestResult result = requestWithRetries(url, "GET", QByteArray(),
                                              QByteArray(), true, nullptr,
                                              timeoutMs, maxRetry, retryDelayMs);
    if (!requestFailed(result)) {
        QJsonParseError parseError{};
        QJsonDocument doc = QJsonDocument::fromJson(result.data, &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            return doc.object().toVariantMap();
        }
    }
    return QMap<QString, QVariant>();
}

bool NGRequest::getFile(const QString &url, const QString &path)
{
    MUTEX_LOCKER;
    instance().resetError();

    const int timeoutMs = std::max(1, instance().m_timeout.toInt()) * 1000;
    const int maxRetry = std::max(0, instance().m_maxRetry.toInt());
    const int retryDelayMs = std::max(0, instance().m_retryDelay.toInt()) * 1000;
    RequestResult result = requestWithRetries(url, "GET", QByteArray(),
                                              QByteArray(), true, nullptr,
                                              timeoutMs, maxRetry, retryDelayMs);
    if (requestFailed(result)) {
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        instance().setErrorMessage(
                    QStringLiteral("Failed to open file '%1' for writing: %2")
                    .arg(path, file.errorString()));
        return false;
    }
    file.write(result.data);
    file.close();

    return true;
}

NGRequest &NGRequest::instance()
{
    static NGRequest n;
    return n;
}

void NGRequest::addAuth(const QString &url, QSharedPointer<IHTTPAuth> auth)
{
    MUTEX_LOCKER;
    m_auths[url] = auth;
}

void NGRequest::removeAuth(const QString &url, const QString &logoutUrl)
{
    MUTEX_LOCKER;

    if (!logoutUrl.isEmpty()) {
        auto prop = properties(url);
        if (!prop.empty()) {
            const QString payload = QString("client_id=%1&refresh_token=%2")
                    .arg(QString::fromUtf8(QUrl::toPercentEncoding(prop["clientId"])),
                         QString::fromUtf8(QUrl::toPercentEncoding(prop["updateToken"])));
            const int timeoutMs = std::max(1, m_timeout.toInt()) * 1000;
            const int maxRetry = std::max(0, m_maxRetry.toInt());
            const int retryDelayMs = std::max(0, m_retryDelay.toInt()) * 1000;
            RequestResult result = requestWithRetries(logoutUrl, "POST",
                                                      payload.toUtf8(),
                                                      "application/x-www-form-urlencoded",
                                                      false, nullptr, timeoutMs,
                                                      maxRetry, retryDelayMs);
            if (requestFailed(result)) {
                qDebug() << "Failed to logout.";
            }
        }
    }
    m_auths.remove(url);
}

bool NGRequest::addAuthURLImpl(const QString &basicUrl, const QString &newUrl)
{
    MUTEX_LOCKER;

    auto it = m_auths.find(basicUrl);
    if (it != m_auths.end()) {
        addAuth(newUrl, it.value());
        return true;
    }
    return false;
}

void NGRequest::removeAuthURLImpl(const QString &url)
{
    MUTEX_LOCKER;
    m_auths.remove(url);
}

const QString NGRequest::authHeader(const QString &url)
{
    MUTEX_LOCKER;

    if (!m_auths.empty() && url == "any") {
        auto it = m_auths.constBegin();
        return it.value()->header();
    }

    auto removeScheme = [](const QString &url) -> QString
    {
        return QUrl(url).toString(QUrl::RemoveScheme);
    };

    QMap<QString, QSharedPointer<IHTTPAuth>>::iterator it;
    for (it = m_auths.begin(); it != m_auths.end(); ++it) {
        if (removeScheme(url).startsWith(removeScheme(it.key()))) {
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
    MUTEX_LOCKER;
    QMap<QString, QString> out;
    if (m_auths.contains(url)) {
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
    MUTEX_LOCKER;

    instance().resetError();
    
    QFile *file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly)) {
        instance().setErrorMessage(QString("Failed to open file: %1").arg(path));
        file->deleteLater();
        return "";
    }

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart filePart;
    const QString fileName = QFileInfo(path).fileName();
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant(QString("form-data; name=\"%1\"; filename=\"%2\"")
                                .arg(name, fileName)));
    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);

    const int timeoutMs = std::max(1, instance().m_timeout.toInt()) * 1000;
    const int maxRetry = 0;
    const int retryDelayMs = std::max(0, instance().m_retryDelay.toInt()) * 1000;
    RequestResult result = requestWithRetries(url, "POST", QByteArray(),
                                              QByteArray(), true, multiPart,
                                              timeoutMs, maxRetry, retryDelayMs);

    if (requestFailed(result)) {
        instance().setErrorMessage(
                    QString("Upload failed. Error: %1. HTTP status: %2. Timed out: %3")
                    .arg(result.errorString)
                    .arg(result.httpStatus)
                    .arg(result.timedOut ? "true" : "false"));
        return "";
    }

    return QString::fromUtf8(result.data);
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
                         int proxyPort, const QString &proxyUser,
                         const QString &proxyPassword, const QString &proxyAuth)
{
    MUTEX_LOCKER;

    if (useProxy) {
        if (useSystemProxy) {
            QNetworkProxyFactory::setUseSystemConfiguration(true);
            // Drop previously forced proxy and return to default/system resolution.
            networkManager()->setProxy(QNetworkProxy::DefaultProxy);
            networkManager()->setProxyFactory(nullptr);
        }
        else {
            QNetworkProxy proxy(QNetworkProxy::HttpProxy, proxyUrl, proxyPort,
                                proxyUser, proxyPassword);
            networkManager()->setProxyFactory(nullptr);
            networkManager()->setProxy(proxy);

            CPLSetConfigOption("GDAL_PROXY_AUTH", proxyAuth.toStdString().c_str());
        }

        const auto gdalProxyCredentials = resolveGDALProxyCredentials(useSystemProxy, proxyUrl, proxyPort, proxyUser, proxyPassword);
        CPLSetConfigOption("GDAL_HTTP_PROXY", gdalProxyCredentials.first.c_str());
        CPLSetConfigOption("GDAL_HTTP_PROXYUSERPWD", gdalProxyCredentials.second.c_str());
    }
    else {
        networkManager()->setProxyFactory(nullptr);
        networkManager()->setProxy(QNetworkProxy::NoProxy);

        CPLSetConfigOption("GDAL_HTTP_PROXY", nullptr);
        CPLSetConfigOption("GDAL_HTTP_PROXYUSERPWD", nullptr);
        CPLSetConfigOption("GDAL_PROXY_AUTH", nullptr);
    }
}
