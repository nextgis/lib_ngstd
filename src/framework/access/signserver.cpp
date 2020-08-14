/******************************************************************************
*  Project: NextGIS GIS libraries
*  Purpose: Framework library
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
#include "signserver.h"
#include "access.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QTcpSocket>
#include <QThread>
#include <QUrl>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif // QT_VERSION >= 0x050000

#include <openssl/rand.h>
#include <openssl/sha.h>

#include <iomanip>
#include <sstream>

#include "core/core.h"

#include "framework/sentryreporter.h"

constexpr const char *msgColor = "#eef5fd";
constexpr const char *errColor = "#fdeeee";
constexpr const char *contentStr = "<html>"
"<head><meta charset=\"UTF-8\"><title>%1</title></head>"
"<body>"
"  <style>"
"    #message{ position: absolute; top: 0; right: 0; left: 0; bottom: 0; margin: auto; height: 226px; width: 680px; font-family: Roboto, Arial, sans-serif; text-align: center; line-height: 1.35; border-radius: 8px; background: %2; padding: 20px 40px; box-sizing: border-box; font-size: 16px; border: 1px solid #d3e3f2; }"
"    #message h1{ font-weight: normal; letter-spacing: -.4px; }"
"    #message span{ color: #0070c5; }"
"  </style>"
"  <div id=\"message\">"
"       <h1>%3</h1>"
"       <p>%4</p>"
"  </div>"
"</body>"
"</html>";

constexpr unsigned short listenPort = 65020;
constexpr const char *redirectUriStr = "http://127.0.0.1:65020";


static std::string toHex(unsigned char *value, int size)
{
    std::ostringstream out;

    for(int i = 0; i < size; ++i) {
        out << std::hex << std::setfill('0') << std::setw(2) <<
               static_cast<unsigned>(value[i]);
    }

    return out.str();
}

static std::string random(int size)
{
    unsigned char *key = new unsigned char[size];
    int rc = RAND_bytes(key, size);
    if (rc != 1) {
        return "";
    }

    return toHex(key, size);
}

static QString generateVerifyCode() {
    return QString::fromStdString(random(32));
}

static QString sha256(const QString &code) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, code.toStdString().c_str(), code.size());
    SHA256_Final(hash, &sha256);
    return toBase64(hash, SHA256_DIGEST_LENGTH);
    //return QString::fromStdString(toHex(hash, SHA256_DIGEST_LENGTH));
}

NGSignServer::NGSignServer(const QString &clientId, const QString &scope,
                           QWidget *parent) :
    QProgressDialog(parent),
    m_redirectUri(redirectUriStr),
    m_clientId(clientId),
    m_scope(scope)
{
    setLabelText(tr("Please sign in\nvia the opened browser..."));
    setWindowModality(Qt::ApplicationModal);
    setMaximum(0);

    if(NGAccess::instance().useCodeChallenge()) {
        m_verifier = generateVerifyCode();
    }

    // Start listen server
    m_listenServer = new QTcpServer(this);
    bool result = m_listenServer->listen(QHostAddress::LocalHost, listenPort);
    if(!result) {
        SentryReporter::instance().sendMessage(QString("NGSignServer listen: 127.0.0.1:%1 result: %2")
                .arg(QString::number(listenPort), result ? "ok" : "bad"), SentryReporter::Level::Error);
    }

    connect(m_listenServer, SIGNAL(newConnection()), this, SLOT(onIncomingConnection()));
}

NGSignServer::~NGSignServer()
{
    m_listenServer->close();
}

QString NGSignServer::code() const
{
    return m_code;
}

QString NGSignServer::redirectUri() const
{
    return m_redirectUri;
}

QString NGSignServer::verifier() const
{
    return m_verifier;
}

void NGSignServer::onIncomingConnection()
{
    QTcpSocket *socket = m_listenServer->nextPendingConnection();
    connect(socket, SIGNAL(readyRead()), this, SLOT(onGetReply()), Qt::UniqueConnection);
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));

//    SentryReporter::instance().sendMessage(QString("NGSignServer::onIncomingConnection %1:%2")
//            .arg(socket->peerAddress().toString(), QString::number(socket->peerPort())));
}

void NGSignServer::onGetReply()
{
    if (!m_listenServer->isListening()) {
        SentryReporter::instance().sendMessage("NGSignServer::onGetReply. Server is not listening",
                                               SentryReporter::Level::Error);
        return;
    }

    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        SentryReporter::instance().sendMessage("NGSignServer::onGetReply. Socket is null",
                                               SentryReporter::Level::Error);
        return;
    }
    socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

    QByteArray data = socket->readAll();
    int result = 0;
    QString dataStr(data);
    QString errorMsg;
    auto lines = dataStr.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
    auto params = lines[0].split(QRegExp("[\\s?&]"),QString::SkipEmptyParts);
    for(const auto &param : params) {
        if(param.startsWith("code=")) {
            m_code = param.mid(5);
            result = m_code.isEmpty() ? 0 : 1;
            break;
        }
        if(param.startsWith("error_description=")) {
            errorMsg = unescapeUrl(param.mid(18));
            result = 0;
            break;
        }
    }


    QByteArray reply;
    reply.append("HTTP/1.0 200 OK \r\n");
    reply.append("Content-Type: text/html; charset=\"utf-8\"\r\n");

    // Add localized content.
    QString messageBody;
    if(result ==1) {
        messageBody = tr("You have successfully signed in <br> at <span>%1</span> application")
            .arg(qApp->applicationName());
    }
    else {
        messageBody = tr("Failed to signed in <br> at <span>%1</span> application.<br>Error: %2")
            .arg(qApp->applicationName()).arg(errorMsg);
    }

    QString replyContent = QString(contentStr)
            .arg(result == 1 ? tr("Successfully signed") : tr("Failed to signed"))
            .arg(result == 1 ? msgColor : errColor)
            .arg(messageBody)
            .arg(tr("Now you can close this page"));

    QByteArray msg = replyContent.toUtf8();
    reply.append(QString("Content-Length: %1\r\n\r\n")
                 .arg(msg.size()).toLatin1());
    reply.append(msg);
    socket->write(reply);
    socket->flush();
    socket->waitForBytesWritten();

    socket->disconnectFromHost();

    if(result == 0) {
    SentryReporter::instance().sendMessage(QString("NGSignServer::onGetReply: data - %1, error - %2")
            .arg(dataStr).arg(errorMsg));
    }

    // Close dialog
    done(result);
}

int NGSignServer::exec()
{
    // Prepare url
    QUrl url(NGAccess::instance().authEndpoint());
    QList<QPair<QString, QString> > parameters;
    parameters.append(qMakePair(QString("response_type"), QString("code")));
    parameters.append(qMakePair(QString("client_id"), m_clientId));
    parameters.append(qMakePair(QString("redirect_uri"), m_redirectUri));
    if(!m_scope.isEmpty()) {
        parameters.append(qMakePair(QString("scope"), m_scope));
    }
    if(!m_verifier.isEmpty()) {
        auto cc = sha256(m_verifier);
        qDebug() << "code_challenge: " << cc;
        parameters.append(qMakePair(QString("code_challenge"), cc));
        parameters.append(qMakePair(QString("code_challenge_method"), QString("S256")));
    }

#if QT_VERSION < 0x050000
    url.setQueryItems(parameters);
#else
    QUrlQuery query(url);
    query.setQueryItems(parameters);
    url.setQuery(query);
#endif

    // Open external browser
//    qDebug() << url.toString();
    bool result = QDesktopServices::openUrl(url);
    if(!result) {
        SentryReporter::instance().sendMessage(QString("QDesktopServices::openUrl %1, result: %2")
            .arg(url.toString(), result ? "ok" : "bad"), SentryReporter::Level::Error);
    }

    return QProgressDialog::exec();
}
