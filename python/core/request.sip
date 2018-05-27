%Import QtCore/QtCoremod.sip

class NGRequest
{
%TypeHeaderCode
#include <request.h>
%End

public:
    static bool addAuth(const QString &url, const QMap<QString, QString> &options);
    static QMap<QString, QVariant> getJsonAsMap(const QString &url);
    static void getFile(const QString &url, const QString &path);
    static QString getAuthHeader(const QString &url);

private:
    NGRequest();
    ~NGRequest();
    NGRequest(const NGRequest &);
    NGRequest &operator=(const NGRequest &);
};
