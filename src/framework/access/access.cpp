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
    QString lang = QCoreApplication::translate("Application", "APP_LANGUAGE");
    QString pricingLink;
    if(lang == "ru") {
        pricingLink = "<a href=\"http://nextgis.ru/pricing/\">http://nextgis.ru/pricing/</a>";
    }
    else {
        pricingLink = "<a href=\"http://nextgis.com/pricing/\">http://nextgis.com/pricing/</a>";
    }
    if (!instance().isUserAuthorized()) {
        QMessageBox::warning(parent, QCoreApplication::translate("Application", "Unsupported"),
                             QCoreApplication::translate("Application",
                                                         "Please upgrade and sign in to use this feature.<br>"
                                                         "View pricing at %1").arg(pricingLink));
    }
    else {
        QMessageBox::warning(parent, QCoreApplication::translate("Application", "Unsupported"),
                             QCoreApplication::translate("Application",
                                                         "Please upgrade to use this feature.<br>"
                                                         "View pricing at %1").arg(pricingLink));
    }
}

NGAccess::NGAccess() :
    m_authorized(false),
    m_supported(false),
    m_avatar(QIcon(":/icons/person-blue.svg"))
{
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

    m_configDir = QString( "%1.config%2access%3" )
            .arg( QDir::homePath() + QDir::separator() )
            .arg( QDir::separator() + QLatin1String(VENDOR) + QDir::separator() )
            .arg( QDir::separator() + QString(QCryptographicHash::hash(
                                                  m_clientId.toLatin1(),
                                                  QCryptographicHash::Md5).toHex()));

    if(!QDir(m_configDir).exists()) {
        QDir().mkdir(m_configDir);
    }

    // Get user id from config
    QString settingsFilePath = m_configDir + QDir::separator() + QLatin1String(settingsFile);
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    QString ngUsertId = settings.value("user_id").toString();
    m_authorized = !ngUsertId.isEmpty();

    if(m_authorized) {
        // Get user avatar from local folder
        QString avatarFilePath = m_configDir + QDir::separator() +
                QLatin1String(avatarFile);
        if(QFileInfo(avatarFilePath).exists()) {
            m_avatar = QIcon(avatarFilePath);
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
            options["clientId"] = m_clientId;
            options["tokenServer"] = tokenEndpoint;
            options["expiresIn"] = settings.value("expires_in").toString();

            QString refreshToken = settings.value("update_token").toString();

            options["accessToken"] = accessToken;
            options["updateToken"] = refreshToken;

            if(!NGRequest::addAuth(apiEndpoint, options)) {
                qDebug() << "Add tokens to NGRequest failed";
            }
        }

        // Load stored plan file
        m_supported = checkSupported();

        // Reguest updates user and support info
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

bool NGAccess::checkSupported()
{
    // Read user id, start/end dates, account type
    QString userId, startDate, endDate, accountType("true"), sign;
    QString settingsFilePath = m_configDir + QDir::separator() + QLatin1String(settingsFile);
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    bool supported = settings.value("supported").toBool();
    if(!supported) {
        return false;
    }

    userId = settings.value("user_id").toString();
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
        return false;
    }

    QDate start = QDate::fromString(startDate, "yyyy-MM-dd");
    QDate end = QDate::fromString(endDate, "yyyy-MM-dd");
    return QDate::currentDate() >= start && QDate::currentDate() <= end;
}

bool NGAccess::verifyRSASignature(unsigned char *originalMessage,
                                  unsigned int messageLength,
                                  unsigned char *signature,
                                  unsigned int sigLength) const
{
    if(nullptr == originalMessage) {
        qWarning() << "Message is empty";
        return false;
    }

    if(nullptr == signature) {
        qWarning() << "Signature is empty";
        return false;
    }

    // https://gist.github.com/sakamoto-poteko/396f289682089e1d767e
    // https://stackoverflow.com/questions/9465727/convert-qfile-to-file
    QString keyFilePath = m_configDir + QDir::separator() + QLatin1String(keyFile);

    FILE *file = fopen(keyFilePath.toLatin1().data(), "r");
    if (!file) {
        qWarning() << tr("Failed open file %1").arg(keyFilePath);
        return false;
    }

    EVP_PKEY *evp_pubkey = PEM_read_PUBKEY(file, nullptr, nullptr, nullptr);
    if (!evp_pubkey) {
        qWarning() << "Failed PEM_read_PUBKEY";
        fclose(file);
        return false;
    }

    fclose(file);

    EVP_MD_CTX *ctx = EVP_MD_CTX_create();
    if (!ctx) {
        qWarning() << "Failed EVP_MD_CTX_create";
        EVP_PKEY_free(evp_pubkey);
        return false;
    }

    if(!EVP_VerifyInit(ctx, EVP_sha256())) {
        EVP_MD_CTX_destroy(ctx);
        EVP_PKEY_free(evp_pubkey);
        qWarning() << "Failed EVP_VerifyInit";
    }

    if(!EVP_VerifyUpdate(ctx, originalMessage, messageLength)) {
        EVP_MD_CTX_destroy(ctx);
        EVP_PKEY_free(evp_pubkey);
        qWarning() << "Failed EVP_VerifyUpdate";
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

extern void updateUserInfoFunction(const QString &configDir)
{
    auto result = NGRequest::getJsonAsMap(QString("%1/user_info/").arg(apiEndpoint));
    QString firstName = result["first_name"].toString();
    QString lastName = result["last_name"].toString();
    QString email = result["email"].toString();
    QString userId = result["nextgis_guid"].toString();

    QString settingsFilePath = configDir + QDir::separator() + QLatin1String(settingsFile);
    QSettings settings(settingsFilePath, QSettings::IniFormat);

    settings.setValue("user_id", userId);
    settings.setValue("first_name", firstName);
    settings.setValue("last_name", lastName);

    // Get avatar
    QString emailHash = QString(QCryptographicHash::hash(
                                    email.toLower().toLatin1(),
                                    QCryptographicHash::Md5).toHex());
    QString avatarPath = configDir + QDir::separator() + QLatin1String(avatarFile);
    NGRequest::getFile(QString("https://www.gravatar.com/avatar/%1?s=64&r=pg&d=robohash")
                       .arg(emailHash), avatarPath);
}

extern void updateSupportInfoFunction(const QString &configDir)
{
    auto result = NGRequest::getJsonAsMap(QString("%1/support_info/").arg(apiEndpoint));
    bool supported = result["supported"].toBool();

    QString settingsFilePath = configDir + QDir::separator() + QLatin1String(settingsFile);
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    settings.setValue("supported", supported);

    if(supported) {
        settings.setValue("sign", result["sign"].toString());
        settings.setValue("start_date", result["start_date"].toString());
        settings.setValue("end_date", result["end_date"].toString());

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
        QString avatarFilePath = m_configDir + QDir::separator() +
                QLatin1String(avatarFile);
        if(QFileInfo(avatarFilePath).exists()) {
            m_avatar = QIcon(avatarFilePath);
            if(m_avatar.isNull()) {
                m_avatar = QIcon(":/icons/person-red.svg");
            }
        }

        m_firstName = settings.value("first_name").toString();
        m_lastName = settings.value("last_name").toString();
    }
    emit userInfoUpdated();
}

void NGAccess::onSupportInfoUpdated()
{
    qDebug() << "onSupportInfoUpdated";
    m_supported = checkSupported();
    emit supportInfoUpdated();
}

void NGAccess::updateUserInfo() const
{
    QFuture<void> future = QtConcurrent::run(updateUserInfoFunction, m_configDir);
    m_updateUserInfoWatcher->setFuture(future);
}

void NGAccess::updateSupportInfo() const
{
    QFuture<void> future = QtConcurrent::run(updateSupportInfoFunction, m_configDir);
    m_updateSupportInfoWatcher->setFuture(future);
}
