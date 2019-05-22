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

#include <QByteArray>

#if QT_VERSION >= 0x050000
    #include <QtConcurrent/QtConcurrent>
#else
    #include <qtconcurrentrun.h>
#endif // QT_VERSION >= 0x050000

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QMainWindow>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>
#include <QTextStream>

#include <openssl/pem.h>
#include <openssl/rsa.h>

#include "request.h"
#include "signserver.h"
#include "version.h"

constexpr const char *apiEndpoint = "https://my.nextgis.com/api/v1";
constexpr const char *tokenEndpoint = "https://my.nextgis.com/oauth2/token/";

constexpr const char *avatarFile = "avatar";
constexpr const char *keyFile = "public.key";
constexpr const char *settingsFile = "settings.ini";

#include "sign.h"

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
    QString pricingLink(tr("<a href=\"http://nextgis.com/pricing/\">Learn more ...</a>"));

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
    m_avatar(QIcon(":/icons/person-blue.svg"))
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
}

QIcon NGAccess::avatar() const
{
    return m_avatar;
}

QString NGAccess::avatarFilePath() const
{
    return  m_configDir + QDir::separator() +
            QLatin1String(avatarFile);
}

QString NGAccess::firstName() const
{
    return m_firstName;
}

QString NGAccess::lastName() const
{
    return m_lastName;
}

void NGAccess::setClientId(const QString &clientId)
{
    m_clientId = clientId;

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
        QDir().mkdir(m_configDir);
    }

    m_logFile.setFileName(m_configDir + QDir::separator() + "access.log");
    m_logFile.open(QIODevice::Append | QIODevice::Text);

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

        // Get access, refresh tokens for network requests
        QString accessToken = settings.value("access_token").toString();

        if(!accessToken.isEmpty()) {

            QMap<QString, QString> options;
            options["type"] = "bearer";
            options["clientId"] = m_clientId;
            options["tokenServer"] = tokenEndpoint;
            options["expiresIn"] = settings.value("expires_in").toString();

            QString refreshToken = settings.value("update_token").toString();

            options["accessToken"] = accessToken;
            options["updateToken"] = refreshToken;

            if(!NGRequest::addAuth(apiEndpoint, options)) {
                qDebug() << "Add tokens to NGRequest failed";
                logMessage("Add tokens to NGRequest failed");
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

void NGAccess::setScope(const QString &scope)
{
    m_scope = scope;
}

void NGAccess::authorize()
{
    // Show modal dialog with cancel button
    NGSignServer listenServer(m_clientId, m_scope);
    listenServer.exec();

    getTokens(listenServer.code(), listenServer.redirectUri());
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

    emit userInfoUpdated();
    emit supportInfoUpdated();
}

void NGAccess::save()
{
    auto properties = NGRequest::instance().properties(apiEndpoint);
    QString settingsFilePath = m_configDir + QDir::separator() + QLatin1String(settingsFile);
    QSettings settings(settingsFilePath, QSettings::IniFormat);

    settings.setValue("expires_in", properties["expiresIn"]);
    settings.setValue("update_token", properties["updateToken"]);
    settings.setValue("access_token", properties["accessToken"]);

    settings.sync();
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

QString NGAccess::getPluginSign(const QString &app, const QString &plugin) const
{
    //TODO: Add check from server
    return signs[app + "." + plugin];
}

bool NGAccess::checkSupported()
{
    // Read user id, start/end dates, account type
    QString userId, startDate, endDate, accountType("true"), sign;
    QString settingsFilePath = m_configDir + QDir::separator() + QLatin1String(settingsFile);
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    bool supported = settings.value("supported").toBool();
    if(!supported) {
        logMessage("Account is not supported");
        return false;
    }

    userId = settings.value("user_id").toString();
    startDate = settings.value("start_date").toString();
    endDate = settings.value("end_date").toString();
    sign = settings.value("sign").toString();

    QString sMessage = userId + startDate + endDate + accountType;

    QByteArray baMessage = sMessage.toUtf8();
    QByteArray baSignature = QByteArray::fromBase64(sign.toUtf8());

    QString errorMsg;
    bool verify = verifyRSASignature(reinterpret_cast<unsigned char*>(baMessage.data()),
                              static_cast<unsigned int>(baMessage.size()),
                              reinterpret_cast<unsigned char*>(baSignature.data()),
                              static_cast<unsigned int>(baSignature.size()),
                              errorMsg);
    if(!verify) {
        logMessage(errorMsg);
        logMessage("Account is supported. Verify failed");
        return false;
    }

    QDate start = QDate::fromString(startDate, "yyyy-MM-dd");
    QDate end = QDate::fromString(endDate, "yyyy-MM-dd");
    bool out = QDate::currentDate() >= start && QDate::currentDate() <= end;
    if(!out) {
        logMessage("Account is supported. Verify success. Period expired.");
    }
    return out;
}

bool NGAccess::verifyRSASignature(unsigned char *originalMessage,
                                  unsigned int messageLength,
                                  unsigned char *signature,
                                  unsigned int sigLength,
                                  QString &errorMsg) const
{
    if(nullptr == originalMessage) {
        qWarning() << "Message is empty";
        errorMsg = "Message is empty";
        return false;
    }

    if(nullptr == signature) {
        qWarning() << "Signature is empty";
        errorMsg = "Signature is empty";
        return false;
    }

    // https://gist.github.com/sakamoto-poteko/396f289682089e1d767e
    // https://stackoverflow.com/questions/9465727/convert-qfile-to-file
    QString keyFilePath = m_configDir + QDir::separator() + QLatin1String(keyFile);
    QFile keyFile(keyFilePath);
    if(!keyFile.open(QIODevice::ReadOnly)) {
        qWarning() << tr("Failed open file %1").arg(keyFilePath);
        errorMsg = QString("Failed open file %1").arg(keyFilePath);
        return false;
    }

    FILE *file = fdopen(keyFile.handle(), "r"); // fopen(keyFilePath.toLatin1().data(), "r");
    if (!file) {
        qWarning() << tr("Failed open file %1").arg(keyFilePath);
        errorMsg = QString("Failed open file %1").arg(keyFilePath);
        return false;
    }

    EVP_PKEY *evp_pubkey = PEM_read_PUBKEY(file, nullptr, nullptr, nullptr);
    if (!evp_pubkey) {
        qWarning() << "Failed PEM_read_PUBKEY";
        errorMsg = "Failed PEM_read_PUBKEY";
        return false;
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_create();
    if (!ctx) {
        qWarning() << "Failed EVP_MD_CTX_create";
        errorMsg = "Failed PEM_read_PUBKEY";
        EVP_PKEY_free(evp_pubkey);
        return false;
    }

    if(!EVP_VerifyInit(ctx, EVP_sha256())) {
        EVP_MD_CTX_destroy(ctx);
        EVP_PKEY_free(evp_pubkey);
        qWarning() << "Failed EVP_VerifyInit";
        errorMsg = "Failed EVP_VerifyInit";
    }

    if(!EVP_VerifyUpdate(ctx, originalMessage, messageLength)) {
        EVP_MD_CTX_destroy(ctx);
        EVP_PKEY_free(evp_pubkey);
        qWarning() << "Failed EVP_VerifyUpdate";
        errorMsg = "Failed EVP_VerifyUpdate";
    }
    int result = EVP_VerifyFinal(ctx, signature, sigLength, evp_pubkey);

    EVP_MD_CTX_destroy(ctx);
    EVP_PKEY_free(evp_pubkey);

    qDebug() << "Signature is " << (result == 1 ? "valid" : "invalid");

    return result == 1;
}

void NGAccess::getTokens(const QString &code, const QString &redirectUri)
{
    if(code.isEmpty()) {
        return;
    }

    qDebug() << "code: " << code << "\nuri:" << redirectUri;

    QMap<QString, QString> options;
    options["type"] = "bearer";
    options["clientId"] = m_clientId;
    options["tokenServer"] = tokenEndpoint;
    options["expiresIn"] = "-1";
    options["code"] = code;
    options["redirectUri"] = redirectUri;

    if(NGRequest::addAuth(apiEndpoint, options)) {
        updateUserInfo();
        updateSupportInfo();

        save();
    }
}

extern void updateUserInfoFunction(const QString &configDir, const QString &licenseDir)
{
    QString firstName, lastName, email, userId;

    // Check local files before request my.nextgis.com
    QFileInfo licenseJson(QDir(licenseDir).filePath("license.json"));
    QMap<QString, QVariant> result;
    if(licenseJson.exists() && licenseJson.isFile()) {
        result = jsonToMap(licenseJson.absoluteFilePath());
    }
    else {
        result = NGRequest::getJsonAsMap(QString("%1/user_info/").arg(apiEndpoint));

    }
    firstName = result["first_name"].toString();
    lastName = result["last_name"].toString();
    email = result["email"].toString();
    userId = result["nextgis_guid"].toString();

    QString settingsFilePath = configDir + QDir::separator() + QLatin1String(settingsFile);
    QSettings settings(settingsFilePath, QSettings::IniFormat);

    settings.setValue("user_id", userId);
    settings.setValue("first_name", firstName);
    settings.setValue("last_name", lastName);

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
        QString emailHash = QString(QCryptographicHash::hash(
                                        email.toLower().toLatin1(),
                                        QCryptographicHash::Md5).toHex());
        NGRequest::getFile(QString("https://www.gravatar.com/avatar/%1?s=64&r=pg&d=robohash")
                           .arg(emailHash), avatarPath);
    }
}

extern void updateSupportInfoFunction(const QString &configDir, const QString &licenseDir)
{
    bool supported = false;
    QString sign, start_date, end_date;
    // Check local files before request my.nextgis.com
    QFileInfo licenseJson(QDir(licenseDir).filePath("license.json"));
    QMap<QString, QVariant> result;
    if(licenseJson.exists() && licenseJson.isFile()) {
        result = jsonToMap(licenseJson.absoluteFilePath());
    }
    else {
        result = NGRequest::getJsonAsMap(QString("%1/support_info/").arg(apiEndpoint));
    }

    supported = result["supported"].toBool();
    sign = result["sign"].toString();
    start_date = result["start_date"].toString();
    end_date = result["end_date"].toString();

    QString settingsFilePath = configDir + QDir::separator() + QLatin1String(settingsFile);
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    settings.setValue("supported", supported);

    if(supported) {
        settings.setValue("sign", sign);
        settings.setValue("start_date", start_date);
        settings.setValue("end_date", end_date);

        // Get key file
        QString keyFilePath = configDir + QDir::separator() + QLatin1String(keyFile);
        NGRequest::getFile(QString("%1/rsa_public_key/").arg(apiEndpoint), keyFilePath);
    }
}

void NGAccess::onUserInfoUpdated()
{
    qDebug() << "onUserInfoUpdated";
    QString settingsFilePath = m_configDir + QDir::separator() + QLatin1String(settingsFile);
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    QString ngUsertId = settings.value("user_id").toString();
    m_authorized = !ngUsertId.isEmpty();
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
    }
    emit userInfoUpdated();

    // If token changed, save
    auto properties = NGRequest::instance().properties(apiEndpoint);
    if(m_updateToken != properties["updateToken"]) {
        save();
    }
}

void NGAccess::onSupportInfoUpdated()
{
    qDebug() << "onSupportInfoUpdated";
    m_supported = checkSupported();
    emit supportInfoUpdated();

    // If token changed, save
    auto properties = NGRequest::instance().properties(apiEndpoint);
    if(m_updateToken != properties["updateToken"]) {
        save();
    }
}

void NGAccess::updateUserInfo() const
{
    auto properties = NGRequest::instance().properties(apiEndpoint);
    m_updateToken = properties["updateToken"];
    QFuture<void> future = QtConcurrent::run(updateUserInfoFunction, m_configDir,
        m_licenseDir);
    m_updateUserInfoWatcher->setFuture(future);
}

void NGAccess::updateSupportInfo() const
{
    auto properties = NGRequest::instance().properties(apiEndpoint);
    m_updateToken = properties["updateToken"];
    QFuture<void> future = QtConcurrent::run(updateSupportInfoFunction, m_configDir,
        m_licenseDir);
    m_updateSupportInfoWatcher->setFuture(future);
}

void NGAccess::logMessage(const QString &value)
{
    QTextStream out(&m_logFile);
    out.setCodec("UTF-8");
    out << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss ") + value << endl;
}
