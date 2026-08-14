/******************************************************************************
 * Project: NextGIS common desktop libraries
 * Purpose: Reusable basic corporate controls
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_BASIC_COMPONENTS_H
#define NGSTD_WIDGETS_BASIC_COMPONENTS_H

#include <ngstd/widgets/types.h>

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QScopedPointer>

class QEvent;
class QPaintEvent;

namespace ngstd {
namespace widgets {

class SearchFieldPrivate;
class IconLabelPrivate;
class ToastPrivate;
class TagPrivate;
class NoticePrivate;

class NGSTD_WIDGETS_EXPORT IconLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(
        ngstd::widgets::IconRole iconRole
        READ iconRole
        WRITE setIconRole
        NOTIFY iconRoleChanged)
    Q_PROPERTY(
        ngstd::widgets::ColorRole colorRole
        READ colorRole
        WRITE setColorRole
        NOTIFY colorRoleChanged)
    Q_PROPERTY(bool iconVisible READ hasIconRole NOTIFY iconVisibilityChanged)
    Q_PROPERTY(
        int iconSize
        READ iconSize
        WRITE setIconSize
        RESET resetIconSize
        NOTIFY iconSizeChanged)

public:
    explicit IconLabel(QWidget *parent = nullptr);
    explicit IconLabel(IconRole iconRole, QWidget *parent = nullptr);
    ~IconLabel() override;

    IconRole iconRole() const;
    bool hasIconRole() const;
    void setIconRole(IconRole iconRole);
    void clearIconRole();

    ColorRole colorRole() const;
    void setColorRole(ColorRole colorRole);

    int iconSize() const;
    void setIconSize(int iconSize);
    void resetIconSize();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void iconRoleChanged(ngstd::widgets::IconRole iconRole);
    void iconVisibilityChanged(bool visible);
    void colorRoleChanged(ngstd::widgets::ColorRole colorRole);
    void iconSizeChanged(int iconSize);

protected:
    void changeEvent(QEvent *event) override;

private:
    NGSTD_WIDGETS_LOCAL void updateIcon();

    Q_DISABLE_COPY(IconLabel)
    QScopedPointer<IconLabelPrivate> d;
};

class NGSTD_WIDGETS_EXPORT SearchField : public QLineEdit
{
    Q_OBJECT

public:
    explicit SearchField(QWidget *parent = nullptr);
    ~SearchField() override;

protected:
    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    NGSTD_WIDGETS_LOCAL void updateSearchIcon();

    Q_DISABLE_COPY(SearchField)
    QScopedPointer<SearchFieldPrivate> d;
};

class NGSTD_WIDGETS_EXPORT Toast : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit Toast(QWidget *parent = nullptr);
    ~Toast() override;

    QString text() const;
    void setText(const QString &text);
    void showMessage(const QString &text, int durationMs = -1);

signals:
    void textChanged(const QString &text);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    NGSTD_WIDGETS_LOCAL void updatePosition();

    Q_DISABLE_COPY(Toast)
    QScopedPointer<ToastPrivate> d;
};

class NGSTD_WIDGETS_EXPORT Tag : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(
        ngstd::widgets::SemanticTone tone
        READ tone
        WRITE setTone
        RESET resetTone
        NOTIFY toneChanged)
    Q_PROPERTY(bool iconVisible READ isIconVisible NOTIFY iconRoleChanged)

public:
    explicit Tag(QWidget *parent = nullptr);
    explicit Tag(const QString &text, QWidget *parent = nullptr);
    ~Tag() override;

    SemanticTone tone() const;
    void setTone(SemanticTone tone);
    void resetTone();

    IconRole iconRole() const;
    bool isIconVisible() const;
    void setIconRole(IconRole role);
    void clearIconRole();

signals:
    void toneChanged(ngstd::widgets::SemanticTone tone);
    void iconRoleChanged(ngstd::widgets::IconRole role, bool visible);

protected:
    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    NGSTD_WIDGETS_LOCAL void initialize();
    NGSTD_WIDGETS_LOCAL void updateIconMargins();

    Q_DISABLE_COPY(Tag)
    QScopedPointer<TagPrivate> d;
};

class NGSTD_WIDGETS_EXPORT Notice : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(
        ngstd::widgets::SemanticTone tone
        READ tone
        WRITE setTone
        RESET resetTone
        NOTIFY toneChanged)

public:
    explicit Notice(QWidget *parent = nullptr);
    ~Notice() override;

    QString title() const;
    void setTitle(const QString &title);

    QString text() const;
    void setText(const QString &text);

    SemanticTone tone() const;
    void setTone(SemanticTone tone);
    void resetTone();

signals:
    void titleChanged(const QString &title);
    void textChanged(const QString &text);
    void toneChanged(ngstd::widgets::SemanticTone tone);

protected:
    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    NGSTD_WIDGETS_LOCAL void updateText();
    NGSTD_WIDGETS_LOCAL void updateIcon();

    Q_DISABLE_COPY(Notice)
    QScopedPointer<NoticePrivate> d;
};

} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_BASIC_COMPONENTS_H
