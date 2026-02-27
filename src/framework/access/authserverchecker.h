#ifndef NGFRAMEWORK_AUTHSERVERCHECKER_H
#define NGFRAMEWORK_AUTHSERVERCHECKER_H

#include <QObject>
#include <QPointer>

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;
class QUrl;

class AuthServerChecker : public QObject
{
    Q_OBJECT
public:
    explicit AuthServerChecker(QObject *parent = nullptr);

    void startCheck(const QUrl &url);

signals:
    void finished(bool available);

private slots:
    void onReplyFinished();
    void onTimeout();

private:
    QNetworkAccessManager *m_networkManager;
    QPointer<QNetworkReply> m_reply;
    QTimer *m_timeoutTimer;
};

#endif // NGFRAMEWORK_AUTHSERVERCHECKER_H

