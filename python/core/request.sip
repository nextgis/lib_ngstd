%Import QtCore/QtCoremod.sip

class NGRequest
{
%TypeHeaderCode
#include <request.h>
%End

public:
    static bool addAuth(const QStringList &urls, const QMap<QString, QString> &options);
    static bool addAuthURL(const QString &basicUrl, const QString &newUrl);
    static void removeAuthURL(const QString &url);
    static QMap<QString, QVariant> getJsonAsMap(const QString &url);
    static QString getJsonAsString(const QString &url);
    static QString getAsString(const QString &url);
    static bool getFile(const QString &url, const QString &path);
    static QString getAuthHeader(const QString &url);
    static QString uploadFile(const QString &url, const QString &path, const QString &name);
    static void setProxy(bool useProxy, bool useSystemProxy, const QString &proxyUrl, int porxyPort, const QString &proxyUser, const QString &proxyPassword, const QString &proxyAuth);
private:
    NGRequest();
    ~NGRequest();
    NGRequest(const NGRequest &);
    NGRequest &operator=(const NGRequest &);
};
