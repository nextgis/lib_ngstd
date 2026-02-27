#include "authserverchecker.h"

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

namespace
{
constexpr auto AuthCheckerTimeoutMs = 10000;
}

AuthServerChecker::AuthServerChecker(QObject *parent) :
    QObject(parent),
    m_networkManager(new QNetworkAccessManager(this)),
    m_reply(nullptr),
    m_timeoutTimer(new QTimer(this))
{
    m_timeoutTimer->setSingleShot(true);
    m_timeoutTimer->setInterval(AuthCheckerTimeoutMs);

    connect(m_timeoutTimer, &QTimer::timeout, this, &AuthServerChecker::onTimeout);
}

void AuthServerChecker::startCheck(const QUrl &url)
{
    if(!url.isValid()) {
        emit finished(false);
        return;
    }

    if(m_reply) {
        m_reply->disconnect(this);
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    QNetworkRequest request(url);
    request.setRawHeader("Accept", "*/*");
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("NextGISAuthCheck/1.0"));

    m_reply = m_networkManager->sendCustomRequest(request, QByteArrayLiteral("OPTIONS"));
    connect(m_reply, &QNetworkReply::finished, this, &AuthServerChecker::onReplyFinished);

    m_timeoutTimer->start();
}

void AuthServerChecker::onReplyFinished()
{
    if(m_timeoutTimer->isActive())
        m_timeoutTimer->stop();

    auto *reply = m_reply.data();
    auto ok = false;

    if(reply) {
        if (reply->error() == QNetworkReply::NoError) {
            const auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            ok = (status >= 200 && status < 400);
        }
        reply->deleteLater();
        m_reply = nullptr;
    }

    emit finished(ok);
}

void AuthServerChecker::onTimeout()
{
    if (m_reply)
    {
        // abort() stops the network request and emits QNetworkReply::finished().
        // As a result, onReplyFinished() will be invoked.
        // In onReplyFinished() we also stop the timeout timer.
        m_reply->abort();
    }
}

