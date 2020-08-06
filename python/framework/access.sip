%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class NGAccess : public QObject
{
%TypeHeaderCode
#include <framework/access/access.h>
%End

public:
    static NGAccess &instance();
    static QIcon lockIcon(const QIcon &origin, const QSize &originSize, const QIcon &lock);
    static void showUnsupportedMessage(QWidget *parent);
    void authorize();
    void exit();
    void save();
    bool isFunctionAvailable(const QString &app, const QString &func) const;
    bool isUserSupported() const;
    bool isUserAuthorized() const;
    QString getPluginSign(const QString &pluginName, const QString &pluginVersion) const;

    void setScope(const QString &scope);
    void setClientId(const QString &clientId);
    void setEndPoint(const QString &endPoint);
    void initSentry(bool enabled, const QString &sentryKey);

    QIcon avatar() const;
    QString avatarFilePath() const;
    QString firstName() const;
    QString lastName() const;
    QString endPoint() const;

signals:
    void userInfoUpdated();
    void supportInfoUpdated();

private:
    NGAccess();
    NGAccess(const NGAccess &);
    NGAccess &operator=(const NGAccess &);
    virtual ~NGAccess();
};
