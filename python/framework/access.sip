%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class NGAccess : public QObject
{
%TypeHeaderCode
#include <framework/access/access.h>
%End

public:
    static NGAccess &instance();
    void authorize();
    void exit();
    void save();
    bool isFunctionAvailable(const QString &app, const QString &func) const;
    bool isUserSupported() const;
    bool isUserAuthorized() const;

    void setScope(const QString &scope);
    void setClientId(const QString &clientId);

    QIcon avatar() const;
    QString avatarFilePath() const;
    QString firstName() const;
    QString lastName() const;

signals:
    void userInfoUpdated();
    void supportInfoUpdated();

private:
    NGAccess();
    NGAccess(const NGAccess &);
    NGAccess &operator=(const NGAccess &);
    virtual ~NGAccess();
};
