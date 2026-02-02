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
#include <QDialog>
#include <QTcpSocket>
#include <QThread>
#include <QUrl>
#include <QTimer>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif // QT_VERSION >= 0x050000

#include <openssl/rand.h>
#include <openssl/sha.h>

#include <iomanip>
#include <sstream>

#include "core/core.h"

#include "logger.h"

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

namespace
{
void logAuth(const LogLevel level, const QString &clientId, const QString &message, const bool flush = false)
{
    auto logger = getLogger();
    const auto payload = QStringLiteral("[Authorization] [%1] %2").arg(clientId, message);

    logger->log(level, payload);

    if (flush)
        logger->flush();
}

constexpr quint16 listenPortStart = 65020;
constexpr quint16 listenPortAttempts = 100;

QString makeRedirectUri(quint16 port)
{
    return QStringLiteral("http://127.0.0.1:%1").arg(port);
}

quint16 listenOnAvailablePort(QTcpServer* server)
{
    if (!server) {
        return 0;
    }

    for (quint16 i = 0; i < listenPortAttempts; ++i) {
        const quint16 port = listenPortStart + i;
        if (server->listen(QHostAddress::LocalHost, port)) {
            return server->serverPort();
        }
        server->close();
    }

    if (server->listen(QHostAddress::LocalHost, 0)) {
        return server->serverPort();
    }

    return 0;
}
}


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
    m_redirectUri(makeRedirectUri(listenPortStart)),
    m_clientId(clientId),
    m_scope(scope),
    m_listenServer(new QTcpServer(this)),
    m_timer(new QTimer(this))
{
    setLabelText(tr("Please sign in\nvia the opened browser..."));
    setWindowModality(Qt::ApplicationModal);
    setMaximum(0);

    if(NGAccess::instance().useCodeChallenge()) {
        m_verifier = generateVerifyCode();
    }

    const auto listeningPort = listenOnAvailablePort(m_listenServer);
    m_listening = listeningPort != 0;

    if (m_listening) {
        m_redirectUri = makeRedirectUri(listeningPort);
        m_listenError.clear();
    } else {
        m_redirectUri.clear();
        m_listenError = m_listenServer->errorString().isEmpty() ? QStringLiteral("Unknown error") : m_listenServer->errorString();
    }

    auto listenMsg = QString("Listen result = %1").arg(m_listening ? "Success" : "Failed");
    listenMsg += m_listening ? QString(", port: %1").arg(listeningPort) : QString(", error: %1").arg(m_listenError);
    logAuth(m_listening ? LogLevel::Debug : LogLevel::Warning, m_clientId, listenMsg);

    if (m_listening)
    {
        connect(m_timer, &QTimer::timeout, this, [this]()
            {
                logAuth(LogLevel::Warning, m_clientId,
                        QStringLiteral("Timeout while waiting for authorization reply"), true);
            });
        m_timer->start(30 * 1000); // 30 sec
    }
    else
    {
        getLogger()->flush();
    }

    connect(m_listenServer, SIGNAL(newConnection()), this, SLOT(onIncomingConnection()));
    connect(m_listenServer, &QTcpServer::acceptError, this, [this](QAbstractSocket::SocketError err)
        {
            m_timer->stop();
            logAuth(LogLevel::Critical, m_clientId,
                    QString("Accept error: %1").arg(QString::number(static_cast<int>(err))), true);
        });
}

NGSignServer::~NGSignServer()
{
    getLogger()->flush();
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

bool NGSignServer::isListening() const
{
    return m_listening;
}

QString NGSignServer::errorString() const
{
    return m_listenError;
}

void NGSignServer::onIncomingConnection()
{
    logAuth(LogLevel::Debug, m_clientId,
            QStringLiteral("Incoming connection received"));

    QTcpSocket *socket = m_listenServer->nextPendingConnection();
    connect(socket, SIGNAL(readyRead()), this, SLOT(onGetReply()), Qt::UniqueConnection);
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
}

void NGSignServer::onGetReply()
{
    m_timer->stop();
    logAuth(LogLevel::Info, m_clientId,
            QStringLiteral("Processing authorization reply"));

    if (!m_listenServer->isListening()) {
        logAuth(LogLevel::Critical, m_clientId,
                QStringLiteral("Authorization server is not listening"), true);
        return;
    }

    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        logAuth(LogLevel::Critical, m_clientId,
                QStringLiteral("Authorization reply socket is null"), true);
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
    if(result == 1) {
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
    socket->deleteLater();

    logAuth(result ? LogLevel::Info : LogLevel::Warning,
            m_clientId,
            QString("Authorization reply status = %1, code = %2, error = %3")
                .arg(result ? "Success" : "Failed", m_code, errorMsg));

    // Close dialog
    done(result);
}

int NGSignServer::exec()
{
    if(!m_listening) {
        logAuth(LogLevel::Critical, m_clientId,
                QStringLiteral("Authorization aborted: listener failed to start"), true);
        return QDialog::Rejected;
    }
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
        getLogger()->debug(QString("code_challenge: %1").arg(cc));
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

    bool result = QDesktopServices::openUrl(url);
    logAuth(result ? LogLevel::Info : LogLevel::Warning,
            m_clientId,
            QString("Open authorization URL status = %1, url = %2")
                .arg(result ? "Success" : "Failed", url.toDisplayString()));

    return QProgressDialog::exec();
}
