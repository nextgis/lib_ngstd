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

#include "access.h"

#include "core/core.h"
#include <QByteArray>
#include "framework/logger//sentrylogger.h"

#if QT_VERSION >= 0x050000
    #include <QtConcurrent/QtConcurrent>
#else
    #include <qtconcurrentrun.h>
#endif // QT_VERSION >= 0x050000

#include <QApplication>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QMainWindow>
#include <QMessageBox>
#include <QUrl>
#include <QUrlQuery>
#include <QPainter>
#include <QSettings>
#include <QTextStream>

#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include "cpl_json.h"

#include "request.h"
#include "signserver.h"
#include "authserverchecker.h"
#include "logger.h"
#include "version.h"
#include <memory>

constexpr const char *apiEndpointSubpath = "/api/v1";
constexpr const char *tokenEndpointSubpath = "/oauth2/token/";
constexpr const char *authEndpointSubpath = "/oauth2/authorize/";
constexpr const char *logoutEndpointSubpath = "/api/v1/logout";

constexpr const char *avatarFile = "avatar";
constexpr const char *keyFile = "public.key";
constexpr const char *settingsFile = "settings.ini";

constexpr const char *defaultScope = "user_info.read";
constexpr const char *defaultEndpoint = "https://my.nextgis.com";
constexpr const char *defaultRedirectUri = "http://127.0.0.1:65020";
constexpr const char *defaultAvatar = ":/icons/person-blue.svg";

static QStringList formOriginsList(NGAccess::AuthSourceType type,
                                   const QString &url1, const QString &url2) {
    QStringList urls;
    if(type == NGAccess::AuthSourceType::NGID) {
        urls << url1 << "https://geoservices.nextgis.com" << "https://nextgis.com" << "https://nextgis.ru";
    }
    else {
        urls << url1 << url2;
    }
    return urls;
}

NGAccess &NGAccess::instance()
{
    static NGAccess s;
    return s;
}

QIcon NGAccess::lockIcon(const QIcon &origin, const QSize &originSize,
                         const QIcon &lock)
{
    QIcon dummyIcon;
    QPixmap comboPixmap(originSize);
    QPixmap firstImage(origin.pixmap(originSize));
    QIcon lockIcon(lock);
    if(lockIcon.isNull()) {
        lockIcon = QIcon(":/icons/lock-icon.svg");
    }
    QSize lockSize(originSize.width() / 1.5, originSize.height() / 1.5);
    QPixmap secondImage(lockIcon.pixmap(lockSize));

    comboPixmap.fill(Qt::transparent);
    QPainter painter(&comboPixmap);
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setBackground(QBrush(Qt::transparent));
    painter.eraseRect(comboPixmap.rect());
    painter.drawPixmap(0, 0, firstImage);
    painter.drawPixmap(originSize.width() - lockSize.width(),
                       originSize.height() - lockSize.height(), secondImage);

    dummyIcon.addPixmap(comboPixmap);
    return dummyIcon;
}

void NGAccess::showUnsupportedMessage(QWidget *parent)
{
    //Get link to pricing for current locale
    QString pricingLink(tr("<a href=\"https://nextgis.com/pricing-base/\">Learn more ...</a>"));

    if (!instance().isUserAuthorized()) {
        QMessageBox::warning(parent, tr("Unsupported"),
                             tr("Please upgrade and sign in to use this feature.<br>"
                                "%1").arg(pricingLink));
    }
    else {
        QMessageBox::warning(parent, tr("Unsupported"),
                             tr("Please upgrade to use this feature.<br>"
                                "%1").arg(pricingLink));
    }
}

NGAccess::NGAccess() :
    m_authorized(false),
    m_supported(false),
    m_endpointAvailable(false),
    m_signInEvent(new SignInEvent(this)),
    m_scope(QLatin1String(defaultScope)),
    m_endpoint(QLatin1String(defaultEndpoint)),
    m_authEndpoint(m_endpoint + authEndpointSubpath),
    m_tokenEndpoint(m_endpoint + tokenEndpointSubpath),
    m_logoutEndpoint(QString()),
    m_authType(AuthSourceType::NGID),
    m_avatar(QIcon(defaultAvatar)),
    m_codeChallenge(false),
    m_authChecker(new AuthServerChecker(this))
{
    // Setup license key file
    QFileInfo appDir(QCoreApplication::applicationDirPath());

#if defined Q_OS_WIN
    m_licenseDir = appDir.dir().absoluteFilePath("share\\license");
#elif defined(Q_OS_MACX)
    // 4 level up
    QDir updaterDir = appDir.dir();
    updaterDir.cdUp();
    updaterDir.cdUp();
    updaterDir.cdUp();
    m_licenseDir = updaterDir.absoluteFilePath("usr/share/license");
#else
    m_licenseDir = QLatin1String("/usr/share/license");
#endif

    m_updateUserInfoWatcher = new QFutureWatcher<void>(this);
    connect(m_updateUserInfoWatcher, SIGNAL(finished()), this,
            SLOT(onUserInfoUpdated()));
    m_updateSupportInfoWatcher = new QFutureWatcher<void>(this);
    connect(m_updateSupportInfoWatcher, SIGNAL(finished()), this,
            SLOT(onSupportInfoUpdated()));

    connect(m_authChecker, &AuthServerChecker::finished,
            this, &NGAccess::onEndpointCheckFinished);
}

QIcon NGAccess::avatar() const
{
    return m_avatar;
}

QString NGAccess::avatarFilePath() const
{
    return m_configDir + QDir::separator() + QLatin1String(avatarFile);
}

QString NGAccess::firstName() const
{
    return m_firstName;
}

QString NGAccess::lastName() const
{
    return m_lastName;
}

QStringList NGAccess::userRoles() const
{
    return m_roles;
}

QObject *NGAccess::getSignInEventFilter()
{
    return m_signInEvent;
}

QString NGAccess::userId() const
{
    return m_userId;
}

QString NGAccess::email() const
{
    return m_email;
}

void NGAccess::setClientId(const QString &clientId)
{
    m_clientId = clientId.trimmed();

    QString config;
#if defined(Q_OS_MACOS) || defined(Q_OS_MAC) // In Qt 4.8 Q_OS_MAC
    config = QLatin1String("Library/Application Support");
#else
    config = QLatin1String(".config");
#endif

    m_configDir = QString( "%1%2access%3" )
            .arg( QDir::homePath() + QDir::separator() + config + QDir::separator())
            .arg( QDir::separator() + QLatin1String(VENDOR) + QDir::separator() )
            .arg( QDir::separator() + QString(QCryptographicHash::hash(
                                                  m_clientId.toLatin1(),
                                                  QCryptographicHash::Md5).toHex()));

    if(!QDir(m_configDir).exists()) {
        QDir().mkpath(m_configDir);
    }

    // Get user id from config
    QString settingsFilePath = m_configDir + QDir::separator() + QLatin1String(settingsFile);
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    QString ngUsertId = settings.value("user_id").toString();
    m_authorized = !ngUsertId.isEmpty();

    if(m_authorized) {
        // Get user avatar from local folder
        if(QFileInfo(avatarFilePath()).exists()) {
            m_avatar = QIcon(avatarFilePath());
            if(m_avatar.isNull()) {
                m_avatar = QIcon(":/icons/person-red.svg");
            }
        }

        m_firstName = settings.value("first_name").toString();
        m_lastName = settings.value("last_name").toString();
        m_userId = settings.value("user_id").toString();
        m_email = settings.value("email").toString();
        m_roles = settings.value("roles").toStringList();

        // Get access, refresh tokens for network requests
        QString accessToken = settings.value("access_token").toString();

        if(!accessToken.isEmpty()) {

            QMap<QString, QString> options;
            options["type"] = "bearer";
            options["clientId"] = m_clientId;
            options["tokenServer"] = m_tokenEndpoint;
            options["expiresIn"] = settings.value("expires_in").toString();

            QString refreshToken = settings.value("update_token").toString();

            options["accessToken"] = accessToken;
            options["updateToken"] = refreshToken;

            QStringList urls = formOriginsList(m_authType, m_endpoint, m_userInfoEndpoint);

            if(!NGRequest::addAuth(urls, options)) {
                logMessage("Add tokens to NGRequest failed", LogLevel::Critical);
            }
        }

        // Load stored plan file
        m_supported = checkSupported();

        // Reguest updates user and support info
        updateUserInfo();
        updateSupportInfo();
    }
    else if(isEnterprise()) {
        updateUserInfo();
        updateSupportInfo();
    }
    else {
        m_avatar = QIcon(":/icons/person-blue.svg");
    }
}

void NGAccess::setAuthEndpoint(const QString &endpoint)
{
    m_authEndpoint = endpoint.trimmed();
}

void NGAccess::setTokenEndpoint(const QString &endpoint)
{
    m_tokenEndpoint = endpoint.trimmed();
}

void NGAccess::setUserInfoEndpoint(const QString &endpoint)
{
    m_userInfoEndpoint = endpoint.trimmed();
}

void NGAccess::setUseCodeChallenge(bool val)
{
    m_codeChallenge = val;
}

void NGAccess::setScope(const QString &scope)
{
    m_scope = scope.trimmed();
}

void NGAccess::setEndPoint(const QString &endPoint, AuthSourceType type)
{
    const auto previousEndpoint = m_endpoint;
    const auto previousType = m_authType;

    if (endPoint.isEmpty()) {
        m_authType = AuthSourceType::NGID;
        m_endpoint = QLatin1String(defaultEndpoint);
        m_avatar = QIcon(defaultAvatar);
        m_tokenEndpoint = m_endpoint + QLatin1String(tokenEndpointSubpath);
        m_authEndpoint = m_endpoint + QLatin1String(authEndpointSubpath);
        m_userInfoEndpoint.clear();
    }
    else {
        m_endpoint = QString(endPoint.trimmed()).remove(QRegularExpression("/+$"));
        m_authType = type;

        if(type == AuthSourceType::NGID) {
            m_tokenEndpoint = m_endpoint + QLatin1String(tokenEndpointSubpath);
            m_authEndpoint = m_endpoint + QLatin1String(authEndpointSubpath);
            m_userInfoEndpoint = QString("%1%2/user_info/").arg(m_endpoint).arg(apiEndpointSubpath);
        }
        else if(type == AuthSourceType::KeyCloakOpenID) {
            m_tokenEndpoint = m_endpoint + QLatin1String("/protocol/openid-connect/token");
            m_authEndpoint = m_endpoint + QLatin1String("/protocol/openid-connect/auth");
            m_logoutEndpoint = m_endpoint + QLatin1String("/protocol/openid-connect/logout");
            m_userInfoEndpoint = m_endpoint + QLatin1String("/protocol/openid-connect/userinfo");
        }
        else if(type == AuthSourceType::Blitz) {
            m_tokenEndpoint = m_endpoint + QLatin1String("/blitz/oauth/te");
            m_authEndpoint = m_endpoint + QLatin1String("/blitz/oauth/ae");
            m_logoutEndpoint = m_endpoint + QLatin1String("/blitz/oauth/logout");
            m_userInfoEndpoint = m_endpoint + QLatin1String("/blitz/oauth/me");
        }
    }

    if(previousEndpoint != m_endpoint || previousType != m_authType) {
        if(m_clientId.isEmpty())
            return;
        checkEndpointAsync();
    }
}

void NGAccess::initSentry(const QString &sentryKey, const QString &version)
{
    auto currentLogger = getLogger();
    auto sentryLogger = std::make_shared<SentryLogger>(
        currentLogger,
        sentryKey,
        version
    );
    setLogger(sentryLogger);
}

QString NGAccess::endPoint() const
{
    return m_endpoint;
}

QString NGAccess::authEndpoint() const
{
    return m_authEndpoint;
}

QString NGAccess::tokenEndpoint() const
{
    return m_tokenEndpoint;
}

QString NGAccess::userInfoEndpoint() const
{
    return m_userInfoEndpoint;
}

bool NGAccess::useCodeChallenge() const
{
    return m_codeChallenge;
}

QUrl NGAccess::buildAuthorizeUrl(const QString &redirectUri,
                                 const QString &codeChallenge,
                                 const QString &codeChallengeMethod,
                                 const QString &authEndpointOverride) const
{
    auto endpoint = authEndpointOverride.trimmed();
    if(endpoint.isEmpty()) {
        endpoint = m_authEndpoint;
    }

    if(endpoint.isEmpty() || m_clientId.isEmpty()) {
        return QUrl();
    }

    QUrl url(endpoint);
    if(!url.isValid()) {
        return QUrl();
    }

    const auto effectiveRedirectUri = redirectUri.trimmed().isEmpty()
            ? QLatin1String(defaultRedirectUri)
            : redirectUri.trimmed();

    QUrlQuery query(url);
    query.addQueryItem(QStringLiteral("response_type"), QStringLiteral("code"));
    query.addQueryItem(QStringLiteral("client_id"), m_clientId);
    query.addQueryItem(QStringLiteral("redirect_uri"), effectiveRedirectUri);

    const auto trimmedScope = m_scope.trimmed();
    if(!trimmedScope.isEmpty())
        query.addQueryItem(QStringLiteral("scope"), trimmedScope);

    if(!codeChallenge.isEmpty()) {
        query.addQueryItem(QStringLiteral("code_challenge"), codeChallenge);
        if(!codeChallengeMethod.isEmpty())
            query.addQueryItem(QStringLiteral("code_challenge_method"), codeChallengeMethod);
    }

    url.setQuery(query);
    return url;
}

enum NGAccess::AuthSourceType NGAccess::authType() const
{
    return m_authType;
}

void NGAccess::authorize()
{
    // Show modal dialog with cancel button
    auto listenServer = std::make_unique<NGSignServer>(m_clientId, m_scope);

    if (listenServer->exec() == QDialog::Rejected && !listenServer->isListening()) {
        const QString errorText = listenServer->errorString();
        listenServer.reset();
        QMessageBox::critical(QApplication::activeWindow(),
            tr("Authorization error"),
            tr("Unable to start local authorization server.\n%1").arg(errorText));
        return;
    }

    getTokens(listenServer->code(), listenServer->redirectUri(), listenServer->verifier());
}

void NGAccess::exit()
{
    m_authorized = m_supported = false;

    QFile file(avatarFilePath());
    file.remove();

    m_avatar = QIcon(":/icons/person-blue.svg");

    QString settingsFilePath = m_configDir + QDir::separator() + QLatin1String(settingsFile);
    QSettings settings(settingsFilePath, QSettings::IniFormat);

    settings.setValue("user_id", "");

    settings.sync();
    // logout
    NGRequest::instance().removeAuth(m_endpoint, m_logoutEndpoint);

    emit userInfoUpdated();
    emit supportInfoUpdated();

}

void NGAccess::save()
{
    auto properties = NGRequest::instance().properties(m_endpoint);
    QString settingsFilePath = m_configDir + QDir::separator() + QLatin1String(settingsFile);
    QSettings settings(settingsFilePath, QSettings::IniFormat);

    settings.setValue("expires_in", properties.value("expiresIn", 0));
    settings.setValue("update_token", properties.value("updateToken", ""));
    settings.setValue("access_token", properties.value("accessToken", ""));

    settings.sync();
}

void NGAccess::checkEndpointAsync()
{
    const auto url = buildAuthorizeUrl(QString(), QString(), QString(), m_authEndpoint);
    if(!url.isValid()) {
        updateEndpointAvailability(false);
        return;
    }

    emit endpointCheckStarted();
    m_authChecker->startCheck(url);
}

void NGAccess::onEndpointCheckFinished(const bool available)
{
    updateEndpointAvailability(available);
}

void NGAccess::updateEndpointAvailability(const bool available)
{
    m_endpointAvailable = available;
    emit endpointAvailableUpdated();
    emit userInfoUpdated();
}

bool NGAccess::isFunctionAvailable(const QString &/*app*/, const QString &/*func*/) const
{
    // TODO: Add more complicated logic which func or app is supported for authorized user
    return isUserSupported();
}

bool NGAccess::isUserSupported() const
{
    return m_supported;
}

bool NGAccess::isUserAuthorized() const
{
    return m_authorized;
}

bool NGAccess::isEnterprise() const
{
    QFileInfo licenseJson(QDir(m_licenseDir).filePath("license.json"));
    return licenseJson.exists() && licenseJson.isFile();
}

bool NGAccess::isEndpointAvailable() const
{
    return m_endpointAvailable;
}

QString NGAccess::getPluginSign(const QString &pluginName, const QString &pluginVersion) const
{
    if(isUserSupported()) {
        return pluginSign(pluginName, pluginVersion);
    }
    return "";
}

bool NGAccess::checkSupported()
{
    if(m_authType != AuthSourceType::NGID) {
        return false;
    }

    // Read user id, start/end dates, account type
    QString userId, startDate, endDate, accountType("true"), sign;
    QString settingsFilePath = m_configDir + QDir::separator() + QLatin1String(settingsFile);
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    bool supported = settings.value("supported").toBool();
    if(!supported) {
        logMessage("Account is not supported", LogLevel::Warning);
        return false;
    }

    userId = settings.value("user_id").toString();
    m_authorized = !userId.isEmpty();
    startDate = settings.value("start_date").toString();
    endDate = settings.value("end_date").toString();
    sign = settings.value("sign").toString();

    QString sMessage = userId + startDate + endDate + accountType;

    QByteArray baMessage = sMessage.toUtf8();
    QByteArray baSignature = QByteArray::fromBase64(sign.toUtf8());

    bool verify = verifyRSASignature(reinterpret_cast<unsigned char*>(baMessage.data()),
                              static_cast<unsigned int>(baMessage.size()),
                              reinterpret_cast<unsigned char*>(baSignature.data()),
                              static_cast<unsigned int>(baSignature.size()));
    if(!verify) {
        logMessage("Account is supported. Verify failed", LogLevel::Critical);
        return false;
    }

    QDate start = QDate::fromString(startDate, "yyyy-MM-dd");
    QDate end = QDate::fromString(endDate, "yyyy-MM-dd");
    bool out = QDate::currentDate() >= start && QDate::currentDate() <= end;
    if(!out) {
        logMessage("Account is supported. Verify success. Period expired.", LogLevel::Warning);
    }
    return out;
}

QString NGAccess::getPublicKey() const
{
    QString keyFilePath = m_configDir + QDir::separator() + QLatin1String(keyFile);
    QFile keyFile(keyFilePath);
    if(!keyFile.open(QFile::ReadOnly | QFile::Text)) {
        return "";
    }
    QTextStream in(&keyFile);
    QString fileText = in.readAll();
    keyFile.close();
    return fileText;
}

bool NGAccess::verifyRSASignature(unsigned char *originalMessage,
                                  unsigned int messageLength,
                                  unsigned char *signature,
                                  unsigned int sigLength
                                  ) const
{
    if(nullptr == originalMessage) {
        getLogger()->warning("Message is empty");
        return false;
    }

    if(nullptr == signature) {
        getLogger()->warning("Signature is empty");
        return false;
    }

    // https://gist.github.com/sakamoto-poteko/396f289682089e1d767e
    // https://stackoverflow.com/questions/9465727/convert-qfile-to-file
    QString keyFilePath = m_configDir + QDir::separator() + QLatin1String(keyFile);
    QFile keyFile(keyFilePath);
    if(!keyFile.open(QIODevice::ReadOnly)) {
        getLogger()->warning(QString("Failed open file %1").arg(keyFilePath));
        return false;
    }

    const QByteArray keyData = keyFile.readAll();
    if (keyData.isEmpty()) {
        getLogger()->warning(QString("Public key file is empty: %1").arg(keyFilePath));
        return false;
    }

    BIO *keyBio = BIO_new_mem_buf(keyData.constData(), static_cast<int>(keyData.size()));
    if (!keyBio) {
        getLogger()->warning("Failed BIO_new_mem_buf for public key");
        return false;
    }

    EVP_PKEY *evp_pubkey = PEM_read_bio_PUBKEY(keyBio, nullptr, nullptr, nullptr);
    BIO_free(keyBio);
    if (!evp_pubkey) {
        getLogger()->warning("Failed PEM_read_bio_PUBKEY");
        return false;
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_create();
    if (!ctx) {
        EVP_PKEY_free(evp_pubkey);
        getLogger()->warning("Failed EVP_MD_CTX_create");
        return false;
    }

    if(!EVP_VerifyInit(ctx, EVP_sha256())) {
        EVP_MD_CTX_destroy(ctx);
        EVP_PKEY_free(evp_pubkey);
        getLogger()->warning("Failed EVP_VerifyInit");
    }

    if(!EVP_VerifyUpdate(ctx, originalMessage, messageLength)) {
        EVP_MD_CTX_destroy(ctx);
        EVP_PKEY_free(evp_pubkey);

        getLogger()->warning("Failed EVP_VerifyUpdate");
    }
    int result = EVP_VerifyFinal(ctx, signature, sigLength, evp_pubkey);

    EVP_MD_CTX_destroy(ctx);
    EVP_PKEY_free(evp_pubkey);

    getLogger()->debug(QString("Signature is %1").arg(result == 1 ? "valid" : "invalid"));

    return result == 1;
}

void NGAccess::getTokens(const QString &code, const QString &redirectUri,
                         const QString &verifier)
{
    if(code.isEmpty()) {
        logMessage("Authorization callback did not contain code", LogLevel::Warning);
        return;
    }

    logMessage(QString("Token request started. tokenEndpoint=%1 redirectUri=%2 codeChallenge=%3")
                   .arg(m_tokenEndpoint,
                        redirectUri,
                        verifier.isEmpty() ? QStringLiteral("disabled") : QStringLiteral("enabled")),
               LogLevel::Debug);

    QMap<QString, QString> options;
    options["type"] = "bearer";
    options["clientId"] = m_clientId;
    options["tokenServer"] = m_tokenEndpoint;
    options["expiresIn"] = "-1";
    options["code"] = code;
    options["redirectUri"] = redirectUri;
    if(!verifier.isEmpty()) {
        options["codeVerifier"] = verifier;
    }

    QStringList urls = formOriginsList(m_authType, m_endpoint, m_userInfoEndpoint);
    if(!NGRequest::addAuth(urls, options)) {
        logMessage(QString("Token request failed. tokenEndpoint=%1").arg(m_tokenEndpoint), LogLevel::Critical);
        return;
    }

    const auto properties = NGRequest::instance().properties(m_endpoint);
    if(properties.value("accessToken").isEmpty()) {
        logMessage(QString("Token request did not return access token. endpoint=%1 tokenEndpoint=%2")
                       .arg(m_endpoint, m_tokenEndpoint), LogLevel::Critical);
        return;
    }

    logMessage("Token request succeeded", LogLevel::Debug);
    updateUserInfo();
    updateSupportInfo();
    save();
}

static QMap<QString, QVariant> userInfoFromJWT(const QString &endPoint) {
    QMap<QString, QVariant> result;
    // Update tokens
    NGRequest::getAuthHeader(endPoint);
    QMap<QString, QString> properties = NGRequest::instance().properties(endPoint);
    QString accessToken = properties["accessToken"];
    QStringList jwtParts = accessToken.split(QLatin1Char('.'), NGSTD_SKIP_EMPTY_PARTS);
    if(jwtParts.size() != 3) {
        return result;
    }
    auto decoded = fromBase64(jwtParts[1]);
    return memJsonToMap(decoded);
}

extern void updateUserInfoFunction(const QString &configDir,
                                   const QString &licenseDir,
                                   const QString &clientId,
                                   const QString &endPoint,
                                   enum NGAccess::AuthSourceType type)
{
    QString firstName, lastName, email, userId, avatarUrl;
    QStringList rolesList;

    // Check local files before request my.nextgis.com
    QFileInfo licenseJson(QDir(licenseDir).filePath("license.json"));
    QMap<QString, QVariant> result;
    if(licenseJson.exists() && licenseJson.isFile()) {
        result = jsonToMap(licenseJson.absoluteFilePath());
    }
    else {
        // Get info from jwt
        result = userInfoFromJWT(endPoint);
        if(result.empty()) {
            result = NGRequest::getJsonAsMap(endPoint);
        }
    }

    if(type == NGAccess::AuthSourceType::NGID) {
        firstName = result["first_name"].toString();
        lastName = result["last_name"].toString();
        email = result["email"].toString();
        userId = result["nextgis_guid"].toString();

        if(email.isEmpty()) {
            email = firstName + QLatin1String(" ") + lastName;
        }

        QString emailHash = QString(QCryptographicHash::hash(
                  email.toLower().toLatin1(), QCryptographicHash::Md5).toHex());

        avatarUrl = QString("https://gravatar.com/avatar/%1?s=64&r=pg&d=robohash")
                .arg(emailHash);
    }
    else if(type == NGAccess::AuthSourceType::KeyCloakOpenID || type == NGAccess::AuthSourceType::Custom || type == NGAccess::AuthSourceType::Blitz) {
        firstName = result["given_name"].toString();
        lastName = result["family_name"].toString();
        email = result["email"].toString();
        userId = result["sub"].toString();
        avatarUrl = result["avatar_url"].toString();
        if(avatarUrl.isEmpty()) {
            QString emailHash = QString(QCryptographicHash::hash(
                      email.toLower().toLatin1(), QCryptographicHash::Md5).toHex());
            avatarUrl = QString("https://gravatar.com/avatar/%1?s=64&r=pg&d=robohash")
                    .arg(emailHash);
        }
        // Get roles
        std::string ra = result["resource_access"].toString().toStdString();
        if(!ra.empty()) {
            CPLJSONDocument doc;
            if(doc.LoadMemory(ra)) {
                auto root = doc.GetRoot();
                auto rolesArray = root.GetArray(clientId.toStdString() + "/roles");
                for(int i = 0; i < rolesArray.Size(); ++i) {
                    rolesList.append(rolesArray[i].ToString().c_str());
                }
            }
        }
    }

    QString settingsFilePath = configDir + QDir::separator() + QLatin1String(settingsFile);
    QSettings settings(settingsFilePath, QSettings::IniFormat);

    if(userId.isEmpty()) {
        NGAccess::instance().logMessage(QString("Get user info map size of %1").arg(result.size()), LogLevel::Debug);
    }

    settings.setValue("user_id", userId);
    settings.setValue("first_name", firstName);
    settings.setValue("last_name", lastName);
    settings.setValue("email", email);
    settings.setValue("roles", rolesList);

    // Get avatar
    QString avatarPath = configDir + QDir::separator() + QLatin1String(avatarFile);
    QFileInfo avatar(QDir(licenseDir).filePath(avatarFile));
    if(avatar.exists() && avatar.isFile()) {
        if(QFile::exists(avatarPath)) {
            QFile::remove(avatarPath);
        }
        QFile::copy(avatar.absoluteFilePath(), avatarPath);
    }
    else {
        NGRequest::getFile(avatarUrl, avatarPath);
    }
}

extern void updateSupportInfoFunction(const QString &configDir,
                                      const QString &licenseDir,
                                      const QString &endPoint)
{
    bool supported = false;
    QString sign, start_date, end_date, userId;
    // Check local files before request my.nextgis.com
    QFileInfo licenseJson(QDir(licenseDir).filePath("license.json"));
    QMap<QString, QVariant> result;
    if(licenseJson.exists() && licenseJson.isFile()) {
        result = jsonToMap(licenseJson.absoluteFilePath());
    }
    else {
        result = NGRequest::getJsonAsMap(QString("%1%2/support_info/").arg(endPoint).arg(apiEndpointSubpath));
    }

    supported = result["supported"].toBool();
    sign = result["sign"].toString();
    start_date = result["start_date"].toString();
    end_date = result["end_date"].toString();
    userId = result["nextgis_guid"].toString();

    QString settingsFilePath = configDir + QDir::separator() + QLatin1String(settingsFile);
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    settings.setValue("supported", supported);
    settings.setValue("user_id", userId);

    if(supported) {
        settings.setValue("sign", sign);
        settings.setValue("start_date", start_date);
        settings.setValue("end_date", end_date);

        QString pkPath = configDir + QDir::separator() + QLatin1String(keyFile);
        QFileInfo pk(QDir(licenseDir).filePath(keyFile));
        if(pk.exists() && pk.isFile()) {
            if(QFile::exists(pkPath)) {
                QFile::remove(pkPath);
            }
            QFile::copy(pk.absoluteFilePath(), pkPath);
        }
        else {
            // Get key file
            QString keyFilePath = configDir + QDir::separator() + QLatin1String(keyFile);
            NGRequest::getFile(QString("%1%2/rsa_public_key/").arg(endPoint).arg(apiEndpointSubpath), keyFilePath);
        }
    }
}

void NGAccess::onUserInfoUpdated()
{
    QString settingsFilePath = m_configDir + QDir::separator() + QLatin1String(settingsFile);
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    QString ngUserId = settings.value("user_id").toString();
    m_authorized = !ngUserId.isEmpty();
    if(m_authorized) {
        if(QFileInfo(avatarFilePath()).exists()) {
            m_avatar = QIcon(avatarFilePath());
            if(m_avatar.isNull()) {
                m_avatar = QIcon(":/icons/person-red.svg");
            }
        }
        else {
            m_avatar = QIcon(":/icons/person-blue.svg");
        }

        m_firstName = settings.value("first_name").toString();
        m_lastName = settings.value("last_name").toString();
        m_userId = settings.value("user_id").toString();
        m_email = settings.value("email").toString();
        m_roles = settings.value("roles").toStringList();
    }

    emit userInfoUpdated();

    // If token changed, save
    auto properties = NGRequest::instance().properties(m_endpoint);
    if(m_updateToken != properties.value("updateToken", "")) {
        save();
    }
}

void NGAccess::onSupportInfoUpdated()
{
    m_supported = checkSupported();

    emit supportInfoUpdated();

    // If token changed, save
    auto properties = NGRequest::instance().properties(m_endpoint);
    if(m_updateToken != properties.value("updateToken", "")) {
        save();
    }
}

void NGAccess::updateUserInfo() const
{
    auto properties = NGRequest::instance().properties(m_endpoint);
    m_updateToken = properties.value("updateToken", "");
    QFuture<void> future = QtConcurrent::run(updateUserInfoFunction, m_configDir,
        m_licenseDir, m_clientId, m_userInfoEndpoint, m_authType);
    m_updateUserInfoWatcher->setFuture(future);
}

void NGAccess::updateSupportInfo() const
{
    if(m_authType != AuthSourceType::NGID) {
        return;
    }
    auto properties = NGRequest::instance().properties(m_endpoint);
    m_updateToken = properties.value("updateToken", "");
    QFuture<void> future = QtConcurrent::run(updateSupportInfoFunction, m_configDir,
        m_licenseDir, m_endpoint);
    m_updateSupportInfoWatcher->setFuture(future);
}

void NGAccess::logMessage(const QString &value, LogLevel level)
{
    auto logger = getLogger();
    const auto payload = QStringLiteral("[NGAccess] %1").arg(value);

    logger->log(level, payload);
}

SignInEvent::SignInEvent(QObject *parent) : QObject(parent)
{
}

bool SignInEvent::eventFilter(QObject *obj, QEvent *event)
{
    NGAccess* ngAccess = qobject_cast<NGAccess*>(parent());

    if (ngAccess && !ngAccess->isUserAuthorized()) {
        if (event->type() == QEvent::Show) {
            ngAccess->checkEndpointAsync();
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}
